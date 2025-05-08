// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "visitor.h"
#include "bytecode-builder.h"

namespace via {

using enum Opcode;
using enum Value::Tag;
using namespace compiler_util;

void StmtNodeVisitor::visit(DeclStmtNode& declaration_node) {
  bool is_global = declaration_node.is_global;
  bool is_const = declaration_node.modifs.is_const;

  ExprNodeBase* val = declaration_node.rvalue;
  TypeNodeBase* val_ty = resolve_type(ctx, val);
  TypeNodeBase* target_ty =
    dynamic_cast<AutoTypeNode*>(declaration_node.type) ? val_ty : declaration_node.type;

  auto& current_closure = get_current_closure(ctx);
  Token ident = declaration_node.identifier;
  symbol_t symbol = ident.lexeme;

  if (is_global) {
    auto previously_declared = ctx.unit_ctx.globals.get_global(symbol);

    if (previously_declared.has_value()) {
      // Error: "global-redeclaration"
      auto message = std::format("Attempt to redeclare global '{}'", symbol);
      compiler_error(ctx, ident, message);
      compiler_output_end(ctx);
    }
    else {
      LitExprNode literal(Token(), symbol);
      Value constant = construct_constant(literal);
      operand_t constant_id = push_constant(ctx, std::move(constant));
      operand_t value_reg = alloc_register(ctx);
      operand_t tmp_reg = alloc_register(ctx);

      CompilerGlobal global{.tok = ident, .symbol = symbol, .type = std::move(val_ty)};
      ctx.unit_ctx.globals.declare_global(std::move(global));

      resolve_rvalue(&expression_visitor, declaration_node.rvalue, value_reg);
      bytecode_emit(ctx, LOADK, {tmp_reg, constant_id});
      bytecode_emit(ctx, SETGLOBAL, {value_reg, tmp_reg}, symbol);
      free_register(ctx, value_reg);
      free_register(ctx, tmp_reg);
    }
  }
  else {
    auto emit_constant =
      [this, &target_ty, &symbol, &current_closure, &declaration_node, &is_const, &val](
        LitExprNode& literal
      ) {
        // Check for Nil
        if (std::get_if<std::monostate>(&literal.value)) {
          bytecode_emit(ctx, PUSHNIL, {}, symbol);
          current_closure.locals.push_back({
            .is_const = is_const,
            .is_constexpr = true,
            .symbol = symbol,
            .decl = &declaration_node,
            .type = target_ty,
            .value = &literal,
          });
        }
        // Check for Int
        else if (int* int_value = std::get_if<int>(&literal.value)) {
          uint32_t final_value = *int_value;
          auto operands = ubit_u32to2u16(final_value);

          bytecode_emit(ctx, PUSHI, {operands.high, operands.low}, symbol);
          current_closure.locals.push_back({
            .is_const = is_const,
            .is_constexpr = true,
            .symbol = symbol,
            .decl = &declaration_node,
            .type = target_ty,
            .value = &literal,
          });
        }
        // Check for float
        else if (float* float_value = std::get_if<float>(&literal.value)) {
          uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
          auto operands = ubit_u32to2u16(final_value);

          bytecode_emit(ctx, PUSHF, {operands.high, operands.low}, symbol);
          current_closure.locals.push_back({
            .is_const = is_const,
            .is_constexpr = true,
            .symbol = symbol,
            .decl = &declaration_node,
            .type = target_ty,
            .value = &literal,
          });
        }
        // Check for Bool
        else if (bool* bool_value = std::get_if<bool>(&literal.value)) {
          bytecode_emit(ctx, *bool_value ? PUSHBT : PUSHBF, {}, symbol);
          current_closure.locals.push_back({
            .is_const = is_const,
            .is_constexpr = true,
            .symbol = symbol,
            .decl = &declaration_node,
            .type = target_ty,
            .value = &literal,
          });
        }
        // Other constant
        else {
          Value constant = construct_constant(dynamic_cast<LitExprNode&>(*val));
          operand_t const_id = push_constant(ctx, std::move(constant));

          bytecode_emit(ctx, PUSHK, {const_id}, symbol);
          current_closure.locals.push_back({
            .is_const = is_const,
            .is_constexpr = true,
            .symbol = symbol,
            .decl = &declaration_node,
            .type = target_ty,
            .value = &literal,
          });
        }
      };

    if (is_constant_expression(ctx.unit_ctx, val)) {
      if (LitExprNode* lit_expr = dynamic_cast<LitExprNode*>(val))
        emit_constant(*lit_expr);
      // Special case: Arrays cannot be represented as LitExprNode.
      else if (dynamic_cast<ArrayExprNode*>(val) != nullptr) {
        val->accept(expression_visitor, OPERAND_INVALID);

        Instruction& bc = ctx.unit_ctx.bytecode.back();
        bc.op = PUSHK;
        bc.a = bc.b;
        bc.b = OPERAND_INVALID;

        current_closure.locals.push_back({
          .is_const = is_const,
          .is_constexpr = false,
          .symbol = symbol,
          .decl = &declaration_node,
          .type = target_ty,
          .value = declaration_node.rvalue,
        });
      }
      else {
        // Constant folding is an O1 optimization.
        if (ctx.unit_ctx.optimization_level < 1) {
          goto non_constexpr;
        }

        LitExprNode literal = fold_constant(ctx, val);
        emit_constant(literal);
      }
    }
    else {
    non_constexpr:
      operand_t dst = alloc_register(ctx);

      resolve_rvalue(&expression_visitor, declaration_node.rvalue, dst);
      bytecode_emit(ctx, PUSH, {dst}, symbol);
      free_register(ctx, dst);

      current_closure.locals.push_back({
        .is_const = is_const,
        .is_constexpr = false,
        .symbol = symbol,
        .decl = &declaration_node,
        .type = target_ty,
        .value = declaration_node.rvalue,
      });
    }
  }

  declaration_node.type->decay(decay_visitor, declaration_node.type);

  // Only do type checking if statement successfully compiled
  if (!failed()) {
    declaration_node.accept(type_visitor);
  }
}

void StmtNodeVisitor::visit(ScopeStmtNode& scope_node) {
  auto& current_closure = get_current_closure(ctx);
  operand_t stack_pointer = current_closure.locals.size();
  ctx.unit_ctx.defered_stmts.push_back({});

  for (StmtNodeBase* stmt : scope_node.statements) {
    stmt->accept(*this);
  }

  std::vector<StmtNodeBase*> defered_stmts = ctx.unit_ctx.defered_stmts.back();
  ctx.unit_ctx.defered_stmts.pop_back();

  // Emit defered statements
  for (StmtNodeBase* stmt : defered_stmts) {
    stmt->accept(*this);
  }

  operand_t stack_allocations = current_closure.locals.size() - stack_pointer;
  for (; stack_allocations > 0; stack_allocations--) {
    bytecode_emit(ctx, DROP);
  }

  current_closure.locals.restore_stack_pointer(stack_pointer);
}

void StmtNodeVisitor::visit(FuncDeclStmtNode& function_node) {
  auto& current_closure = get_current_closure(ctx);
  operand_t function_reg = alloc_register(ctx);

  // Store the function node information we'll need later
  bool is_const = true;
  bool is_constexpr = false;
  auto symbol = function_node.identifier.lexeme;
  auto* decl = &function_node;

  // Create the function type node before any potential invalidation
  auto* function_type = ctx.unit_ctx.ast_allocator.emplace<FunctionTypeNode>(
    function_node.parameters, function_node.returns
  );

  ctx.unit_ctx.function_stack.push_back({
    .stack_pointer = current_closure.locals.size(),
    .decl = &function_node,
    .locals = {},
  });

  function_node.returns->decay(decay_visitor, function_node.returns);
  function_node.accept(type_visitor);

  ctx.unit_ctx.defered_stmts.push_back({});
  bytecode_emit(
    ctx,
    CLOSURE,
    {function_reg, 0, (operand_t)function_node.parameters.size()},
    function_node.identifier.lexeme
  );

  size_t new_closure_point = ctx.unit_ctx.bytecode.size();
  ScopeStmtNode& scope = dynamic_cast<ScopeStmtNode&>(*function_node.body);

  for (StmtNodeBase*& pstmt : scope.statements) {
    const StmtNodeBase& stmt = *pstmt;
    const DeclStmtNode* declaration_node = dynamic_cast<const DeclStmtNode*>(&stmt);
    const FuncDeclStmtNode* function_node = dynamic_cast<const FuncDeclStmtNode*>(&stmt);

    if (declaration_node || function_node) {
      bool is_global = declaration_node ? declaration_node->is_global : function_node->is_global;
      Token identifier =
        declaration_node ? declaration_node->identifier : function_node->identifier;

      if (is_global) {
        // Error: "global-decl-within-function"
        auto message = "Function scopes cannot declare globals";
        compiler_error(ctx, identifier, message);
        compiler_info(
          ctx,
          "Function scopes containing global declarations may cause previously declared "
          "globals to be re-declared, therefore are not allowed."
        );
        compiler_output_end(ctx);
        break;
      }
    }

    pstmt->accept(*this);
  }

  close_defer_statements(ctx, this);

  Instruction& last_bytecode = ctx.unit_ctx.bytecode.back();
  Opcode last_opcode = last_bytecode.op;

  if (last_opcode != RET && last_opcode != RETNIL) {
    bytecode_emit(ctx, RETNIL);
  }

  Instruction& new_closure = ctx.unit_ctx.bytecode.at(new_closure_point - 1);
  new_closure.b = ctx.unit_ctx.bytecode.size() - new_closure_point;

  if (function_node.is_global) {
    if (ctx.unit_ctx.globals.was_declared(symbol)) {
      // Error: "global-redecl"
      auto message = std::format("Redeclaring global '{}'", symbol);
      compiler_error(ctx, function_node.identifier, message);
      compiler_output_end(ctx);
      return;
    }

    LitExprNode literal(Token(), symbol);
    Value constant = construct_constant(literal);
    operand_t constant_id = push_constant(ctx, std::move(constant));
    operand_t tmp_reg = alloc_register(ctx);

    bytecode_emit(ctx, LOADK, {tmp_reg, constant_id});
    bytecode_emit(ctx, SETGLOBAL, {function_reg, tmp_reg});
    free_register(ctx, tmp_reg);
  }
  else {
    bytecode_emit(ctx, PUSH, {function_reg});
  }

  ctx.unit_ctx.function_stack.pop_back();
  auto& updated_closure = get_current_closure(ctx);
  updated_closure.locals.push_back({
    .is_const = is_const,
    .is_constexpr = is_constexpr,
    .symbol = symbol,
    .decl = decl,
    .type = function_type,
    .value = nullptr,
  });

  free_register(ctx, function_reg);
}

void StmtNodeVisitor::visit(AssignStmtNode& assign_node) {
  register_t temp = alloc_register(ctx);
  resolve_rvalue(&expression_visitor, assign_node.rvalue, temp);
  bind_lvalue(ctx, assign_node.lvalue, temp);
  free_register(ctx, temp);

  if (!failed()) {
    type_visitor.visit(assign_node);
  }
}

void StmtNodeVisitor::visit(ReturnStmtNode& return_node) {
  auto& this_function = get_current_closure(ctx);
  if (return_node.expression) {
    operand_t expr_reg = alloc_register(ctx);
    resolve_rvalue(&expression_visitor, return_node.expression, expr_reg);
    bytecode_emit(ctx, RET, {expr_reg}, this_function.decl->identifier.lexeme);
    free_register(ctx, expr_reg);
  }
  else {
    bytecode_emit(ctx, RETNIL, {}, this_function.decl->identifier.lexeme);
  }
}

void StmtNodeVisitor::visit(BreakStmtNode& break_node) {
  if (!ctx.lesc.has_value()) {
    // Error: "ill-break"
    auto message = "'break' statement not within loop or switch";
    compiler_error(ctx, break_node.begin, break_node.end, message);
    compiler_output_end(ctx);
  }
  else {
    bytecode_emit(ctx, LJMP, {*ctx.lesc}, "break");
  }
}

void StmtNodeVisitor::visit(ContinueStmtNode& continue_node) {
  if (!ctx.lesc.has_value()) {
    // Error: "ill-continue"
    auto message = "'continue' statement not within loop or switch";
    compiler_error(ctx, continue_node.begin, continue_node.end, message);
    compiler_output_end(ctx);
  }
  else {
    bytecode_emit(ctx, LJMP, {*ctx.lrep}, "continue");
  }
}

void StmtNodeVisitor::visit(IfStmtNode& if_node) {
  // Handle attributes
  for (const StmtAttribute& attr : if_node.attributes) {
    if (attr.identifier.lexeme == "compile_time") {
      size_t begin, end;

      if (!is_constant_expression(ctx.unit_ctx, if_node.condition)) {
        begin = if_node.condition->begin, end = if_node.condition->end;
        goto conditions_not_constexpr;
      }

      for (const ElseIfNode* elif : if_node.elseif_nodes) {
        if (!is_constant_expression(ctx.unit_ctx, elif->condition)) {
          begin = elif->condition->begin, end = elif->condition->end;
          goto conditions_not_constexpr;
        }
      }

      goto evaluate_if;
    conditions_not_constexpr:
      auto message = "Attribute 'compile_time' on if statement requires all conditions to be a "
                     "constant expression";
      compiler_error(ctx, begin, end, message);
      compiler_info(ctx, attr.identifier, "Attribute 'compile_time' passed here");
      compiler_output_end(ctx);
      return;
    }
    else {
      // Warning: "unused-attribute"
      auto message = std::format("Unused attribute '{}'", attr.identifier.lexeme);
      compiler_warning(ctx, attr.identifier, message);
      compiler_output_end(ctx);
    }
  }

  // Compile-time if-statement evaluation is an O1 optimization unless explicitly specified with
  // attribute '@compile_time'
  if (ctx.unit_ctx.optimization_level >= 1 && is_constant_expression(ctx.unit_ctx, if_node.condition)) {
  evaluate_if:
    auto evaluate_case = [this](ExprNodeBase* condition, StmtNodeBase* scope) -> bool {
      LitExprNode lit = fold_constant(ctx, condition);

      if (std::holds_alternative<std::monostate>(lit.value))
        return false;
      else if (bool* bool_val = std::get_if<bool>(&lit.value))
        if (!(*bool_val))
          return false;

      scope->accept(*this);
      return true;
    };

    if (!evaluate_case(if_node.condition, if_node.scope)) {
      for (const ElseIfNode* elif : if_node.elseif_nodes) {
        if (!evaluate_case(elif->condition, elif->scope))
          continue;
        return;
      }

      if_node.else_node->accept(*this);
    }

    return;
  }

  operand_t cond_reg = alloc_register(ctx);
  operand_t if_label = ctx.unit_ctx.label_count++;

  resolve_rvalue(&expression_visitor, if_node.condition, cond_reg);
  bytecode_emit(ctx, LJMPIF, {cond_reg, if_label}, "if");

  for (const ElseIfNode* else_if : if_node.elseif_nodes) {
    operand_t label = ctx.unit_ctx.label_count++;
    resolve_rvalue(&expression_visitor, else_if->condition, cond_reg);
    bytecode_emit(ctx, LJMPIF, {cond_reg, label}, std::format("elseif #{}", label - if_label));
  }

  free_register(ctx, cond_reg);

  operand_t escape_label = ctx.unit_ctx.label_count++;

  bytecode_emit(ctx, LJMP, {escape_label}, "else");
  bytecode_emit(ctx, LBL, {if_label});
  if_node.scope->accept(*this);
  bytecode_emit(ctx, LJMP, {escape_label});

  size_t label_id = 0;
  for (const ElseIfNode* else_if : if_node.elseif_nodes) {
    operand_t label = if_label + ++label_id;
    bytecode_emit(ctx, LBL, {label});
    else_if->scope->accept(*this);
    bytecode_emit(ctx, LJMP, {escape_label});
  }

  bytecode_emit(ctx, LBL, {escape_label});

  if (if_node.else_node) {
    if_node.else_node->accept(*this);
  }
}

void StmtNodeVisitor::visit(WhileStmtNode& while_node) {
  operand_t repeat_label = ctx.unit_ctx.label_count++;
  operand_t escape_label = ctx.unit_ctx.label_count++;
  operand_t cond_reg = alloc_register(ctx);

  ctx.lrep = repeat_label;
  ctx.lesc = escape_label;

  bytecode_emit(ctx, LBL, {repeat_label});
  resolve_rvalue(&expression_visitor, while_node.condition, cond_reg);
  bytecode_emit(ctx, LJMPIFN, {cond_reg, escape_label});

  while_node.body->accept(*this);

  bytecode_emit(ctx, LJMP, {repeat_label});
  bytecode_emit(ctx, LBL, {escape_label});
  free_register(ctx, cond_reg);

  ctx.lrep = std::nullopt;
  ctx.lesc = std::nullopt;
}

void StmtNodeVisitor::visit(DeferStmtNode& defer_stmt) {
  if (!ctx.unit_ctx.defered_stmts.empty()) {
    auto& top = ctx.unit_ctx.defered_stmts.back();
    top.push_back(defer_stmt.stmt);
  }
}

void StmtNodeVisitor::visit(ExprStmtNode& expr_stmt) {
  ExprNodeBase* expr = expr_stmt.expression;
  operand_t reg = alloc_register(ctx);

  resolve_rvalue(&expression_visitor, expr, reg);

  if (is_nil(expr->infer_type(ctx.unit_ctx)))
    return;
  else if (CallExprNode* call_node = dynamic_cast<CallExprNode*>(expr)) {
    TypeNodeBase* ret_ty = resolve_type(ctx, call_node);

    if (PrimTypeNode* prim_ty = dynamic_cast<PrimTypeNode*>(ret_ty)) {
      if (prim_ty->type != Nil) {
        goto return_value_ignored;
      }
    }
    else {
    return_value_ignored:
      // Warning: "return-value-ignored"
      auto message = "Function return value ignored";
      compiler_warning(ctx, call_node->begin, call_node->end, message);
      compiler_info(ctx, std::format("Function returns type {}", ret_ty->to_output_string()));
      compiler_output_end(ctx);
    }
  }
  else {
    // Warning: "expr-result-unused"
    auto message = "Expression result unused";
    compiler_warning(ctx, expr->begin, expr->end, message);
    compiler_output_end(ctx);
  }

  free_register(ctx, reg);
}

} // namespace via
