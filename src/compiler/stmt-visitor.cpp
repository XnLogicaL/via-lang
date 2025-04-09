//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "bit-utility.h"
#include "string-utility.h"
#include "compiler-types.h"
#include "visitor.h"
#include "stack.h"
#include "format-vector.h"

namespace via {

using enum IOpCode;

void StmtNodeVisitor::visit(DeclStmtNode& declaration_node) {
  bool is_global = declaration_node.is_global;
  bool is_const = declaration_node.modifs.is_const;

  ExprNodeBase& val = *declaration_node.rvalue;
  TypeNodeBase* val_ty = val.infer_type(unit_ctx);
  Token ident = declaration_node.identifier;
  symbol_t symbol = ident.lexeme;

  if (is_global) {
    std::string comment = symbol;
    std::optional<CompilerGlobal> previously_declared =
      unit_ctx.internal.globals->get_global(symbol);

    if (previously_declared.has_value()) {
      compiler_error(ident, std::format("Attempt to re-declare global '{}'", symbol));
      compiler_info(previously_declared->tok, "Previously declared here");
      compiler_output_end();
    }
    else {
      operand_t value_reg = allocator.allocate_register();
      operand_t symbol_hash = hash_string_custom(symbol.c_str());

      CompilerGlobal global{.tok = ident, .symbol = symbol, .type = std::move(val_ty)};
      unit_ctx.internal.globals->declare_global(std::move(global));
      declaration_node.rvalue->accept(expression_visitor, value_reg);
      unit_ctx.bytecode->emit(GSET, {value_reg, symbol_hash}, comment);
      allocator.free_register(value_reg);
    }
  }
  else {
    std::string comment = std::format("{}", symbol);

    if (is_constant_expression(unit_ctx, &val)) {
      // Constant folding is an O1 optimization.
      if (unit_ctx.optimization_level < 1) {
        goto non_constexpr;
      }

      LitExprNode literal = expression_visitor.fold_constant(val);

      // Check for nil
      if (std::get_if<std::monostate>(&literal.value)) {
        unit_ctx.bytecode->emit(PUSHNIL, {}, comment);
        unit_ctx.internal.variable_stack->push({
          .is_const = is_const,
          .is_constexpr = true,
          .symbol = symbol,
          .decl = &declaration_node,
          .type =
            unit_ctx.ast->allocator.emplace<PrimTypeNode>(literal.value_token, IValueType::nil),
          .value = &literal,
        });
      }
      // Check for integer
      else if (int* int_value = std::get_if<int>(&literal.value)) {
        uint32_t final_value = *int_value;
        auto operands = reinterpret_u32_as_2u16(final_value);

        unit_ctx.bytecode->emit(PUSHI, {operands.high, operands.low}, comment);
        unit_ctx.internal.variable_stack->push({
          .is_const = is_const,
          .is_constexpr = true,
          .symbol = symbol,
          .decl = &declaration_node,
          .type =
            unit_ctx.ast->allocator.emplace<PrimTypeNode>(literal.value_token, IValueType::integer),
          .value = &literal,
        });
      }
      // Check for float
      else if (float* float_value = std::get_if<float>(&literal.value)) {
        uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
        auto operands = reinterpret_u32_as_2u16(final_value);

        unit_ctx.bytecode->emit(PUSHF, {operands.high, operands.low}, comment);
        unit_ctx.internal.variable_stack->push({
          .is_const = is_const,
          .is_constexpr = true,
          .symbol = symbol,
          .decl = &declaration_node,
          .type = unit_ctx.ast->allocator.emplace<PrimTypeNode>(
            literal.value_token, IValueType::floating_point
          ),
          .value = &literal,
        });
      }
      // Check for boolean
      else if (bool* bool_value = std::get_if<bool>(&literal.value)) {
        unit_ctx.bytecode->emit(*bool_value ? PUSHBT : PUSHBF, {}, comment);
        unit_ctx.internal.variable_stack->push({
          .is_const = is_const,
          .is_constexpr = true,
          .symbol = symbol,
          .decl = &declaration_node,
          .type =
            unit_ctx.ast->allocator.emplace<PrimTypeNode>(literal.value_token, IValueType::boolean),
          .value = &literal,
        });
      }
      // Other constant
      else {
        const IValue& constant =
          expression_visitor.construct_constant(dynamic_cast<LitExprNode&>(val));
        const operand_t const_id = unit_ctx.constants->push_constant(constant);

        unit_ctx.bytecode->emit(PUSHK, {const_id}, comment);
        unit_ctx.internal.variable_stack->push({
          .is_const = is_const,
          .is_constexpr = true,
          .symbol = symbol,
          .decl = &declaration_node,
          .type = unit_ctx.ast->allocator.emplace<PrimTypeNode>(literal.value_token, constant.type),
          .value = &literal,
        });
      }
    }
    else {
    non_constexpr:
      operand_t dst = allocator.allocate_register();

      declaration_node.rvalue->accept(expression_visitor, dst);
      unit_ctx.bytecode->emit(PUSH, {dst}, comment);
      unit_ctx.internal.variable_stack->push({
        .is_const = is_const,
        .is_constexpr = false,
        .symbol = symbol,
        .decl = &declaration_node,
        .type = std::move(val_ty),
        .value = declaration_node.rvalue,
      });

      allocator.free_register(dst);
    }
  }

  // Decay statement type before type checking
  declaration_node.type->decay(decay_visitor, declaration_node.type);

  // Only do type checking if statement successfully compiled
  if (!failed()) {
    declaration_node.accept(type_visitor);
  }
}

void StmtNodeVisitor::visit(ScopeStmtNode& scope_node) {
  operand_t stack_pointer = unit_ctx.internal.variable_stack->size();

  for (StmtNodeBase*& pstmt : scope_node.statements) {
    pstmt->accept(*this);
  }

  // Emit defered statements
  for (StmtNodeBase* stmt : unit_ctx.internal.defered_stmts) {
    stmt->accept(*this);
  }

  unit_ctx.internal.defered_stmts.clear();

  operand_t stack_allocations = unit_ctx.internal.variable_stack->size() - stack_pointer;
  for (; stack_allocations > 0; stack_allocations--) {
    unit_ctx.bytecode->emit(DROP);
  }
}

void StmtNodeVisitor::visit(FuncDeclStmtNode& function_node) {
  using parameters_t = FuncDeclStmtNode::parameters_t;

  operand_t function_reg = allocator.allocate_register();
  // Fucking move semantics forcing me into doing this
  parameters_t parameters_0;
  parameters_t parameters_1;

  for (const ParamNode& param : function_node.parameters) {
    TypeNodeBase* type_0 = param.type;
    TypeNodeBase* type_1 = param.type;

    type_0->decay(decay_visitor, type_0);
    type_1->decay(decay_visitor, type_1);

    parameters_0.emplace_back(param.identifier, param.modifs, type_0);
    parameters_1.emplace_back(param.identifier, param.modifs, type_1);
  }

  unit_ctx.internal.function_stack->push({
    .stack_pointer = unit_ctx.internal.variable_stack->size(),
    .decl = &function_node,
  });

  unit_ctx.internal.variable_stack->push({
    .is_const = function_node.modifs.is_const,
    .is_constexpr = false,
    .symbol = function_node.identifier.lexeme,
    .decl = &function_node,
    .type = unit_ctx.ast->allocator.emplace<FunctionTypeNode>(parameters_0, function_node.returns),
    .value = nullptr,
  });

  function_node.returns->decay(decay_visitor, function_node.returns);
  function_node.accept(type_visitor);
  unit_ctx.bytecode->emit(NEWCLSR, {function_reg}, function_node.identifier.lexeme);

  size_t new_closure_point = unit_ctx.bytecode->size();
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
        compiler_error(identifier, "Function scopes cannot declare globals");
        compiler_info(
          "Function scopes containing global declarations may cause previously declared "
          "globals to be re-declared, therefore are not allowed."
        );
        compiler_output_end();
        break;
      }
    }

    pstmt->accept(*this);
  }

  // Emit defered statements
  for (StmtNodeBase* stmt : unit_ctx.internal.defered_stmts) {
    stmt->accept(*this);
  }

  unit_ctx.internal.defered_stmts.clear();

  Bytecode& last_bytecode = unit_ctx.bytecode->back();
  IOpCode last_opcode = last_bytecode.instruct.op;

  if (last_opcode != RET && last_opcode != RETNIL) {
    unit_ctx.bytecode->emit(RETNIL);
  }

  Bytecode& new_closure = unit_ctx.bytecode->at(new_closure_point - 1);
  new_closure.instruct.operand1 = unit_ctx.bytecode->size() - new_closure_point;

  Token symbol_token = function_node.identifier;
  symbol_t symbol = symbol_token.lexeme;
  uint32_t symbol_hash = hash_string_custom(symbol.c_str());

  if (function_node.is_global) {
    if (unit_ctx.internal.globals->was_declared(symbol)) {
      compiler_error(symbol_token, std::format("Attempt to re-declare global '{}'", symbol));
      compiler_output_end();
      return;
    }

    auto operands = reinterpret_u32_as_2u16(symbol_hash);
    unit_ctx.bytecode->emit(GSET, {function_reg, operands.high, operands.low});
  }
  else {
    unit_ctx.bytecode->emit(PUSH, {function_reg});
  }

  allocator.free_register(function_reg);
  unit_ctx.internal.function_stack->pop();
}

void StmtNodeVisitor::visit(AssignStmtNode& assign_node) {
  if (SymExprNode* symbol_node =
        get_derived_instance<ExprNodeBase, SymExprNode>(assign_node.lvalue)) {
    Token symbol_token = symbol_node->identifier;
    std::string symbol = symbol_token.lexeme;
    std::optional<operand_t> stk_id = unit_ctx.internal.variable_stack->find_symbol(symbol);

    if (stk_id.has_value()) {
      const auto& test_stack_member = unit_ctx.internal.variable_stack->at(stk_id.value());
      if (test_stack_member.has_value() && test_stack_member->is_const) {
        compiler_error(
          symbol_token, std::format("Assignment to constant lvalue (variable) '{}'", symbol)
        );
        compiler_output_end();
        return;
      }

      operand_t value_reg = allocator.allocate_register();
      assign_node.rvalue->accept(expression_visitor, value_reg);

      if (unit_ctx.internal.function_stack->size() > 0) {
        const auto& current_closure = unit_ctx.internal.function_stack->top();
        if (current_closure.stack_pointer < stk_id.value()) {
          unit_ctx.bytecode->emit(STKSET, {value_reg, stk_id.value()}, symbol);
        }
        else {
          unit_ctx.bytecode->emit(UPVSET, {value_reg, stk_id.value()}, symbol);
        }
      }
    }
    else {
      compiler_error(symbol_token, "Assignment to invalid lvalue");
      compiler_info(std::format("Symbol '{}' not found in scope", symbol_node->identifier.lexeme));
      compiler_output_end();
    }
  }
  else {
    compiler_error(
      assign_node.lvalue->begin, assign_node.lvalue->end, "Assignment to invalid lvalue"
    );
    compiler_output_end();
  }

  if (!failed()) {
    type_visitor.visit(assign_node);
  }
}

void StmtNodeVisitor::visit(ReturnStmtNode& return_node) {
  auto& this_function = unit_ctx.internal.function_stack->top();
  if (return_node.expression) {
    operand_t expr_reg = allocator.allocate_register();

    return_node.expression->accept(expression_visitor, expr_reg);
    unit_ctx.bytecode->emit(RET, {expr_reg}, this_function.decl->identifier.lexeme);
    allocator.free_register(expr_reg);
  }
  else {
    unit_ctx.bytecode->emit(RETNIL, {}, this_function.decl->identifier.lexeme);
  }
}

void StmtNodeVisitor::visit(BreakStmtNode& break_node) {
  if (!escape_label.has_value()) {
    compiler_error(break_node.begin, break_node.end, "'break' statement not within loop or switch");
    compiler_output_end();
  }
  else {
    unit_ctx.bytecode->emit(LJMP, {escape_label.value()}, "break");
  }
}

void StmtNodeVisitor::visit(ContinueStmtNode& continue_node) {
  if (!repeat_label.has_value()) {
    compiler_error(continue_node.begin, continue_node.end, "'continue' statement not within loop");
    compiler_output_end();
  }
  else {
    unit_ctx.bytecode->emit(LJMP, {repeat_label.value()}, "continue");
  }
}

void StmtNodeVisitor::visit(IfStmtNode& if_node) {
  /*

  0000 jumplabelif    0, 0    ; if
  0001 jumplabelif    1, 1    ; elseif #1
  0002 jumplabel      2       ; else
  ...
  0003 label          0       ; if
  ...
  0004 jumplabel      3
  0005 label          1       ; elseif #1
  ...
  0006 jumplabel      3
  0007 label          2       ; else
  ...
  0008 label          3       ; escape

  */
  operand_t cond_reg = allocator.allocate_register();
  operand_t if_label = unit_ctx.internal.label_count++;

  if_node.condition->accept(expression_visitor, cond_reg);
  unit_ctx.bytecode->emit(LJMPIF, {cond_reg, if_label}, "if");

  for (const ElseIfNode* else_if : if_node.elseif_nodes) {
    operand_t label = unit_ctx.internal.label_count++;
    else_if->condition->accept(expression_visitor, cond_reg);
    unit_ctx.bytecode->emit(LJMPIF, {cond_reg, label}, std::format("elseif #{}", label - if_label));
  }

  allocator.free_register(cond_reg);

  operand_t escape_label = unit_ctx.internal.label_count++;

  unit_ctx.bytecode->emit(LJMP, {escape_label}, "else");
  unit_ctx.bytecode->emit(LBL, {if_label});
  if_node.scope->accept(*this);
  unit_ctx.bytecode->emit(LJMP, {escape_label});

  size_t label_id = 0;
  for (const ElseIfNode* else_if : if_node.elseif_nodes) {
    operand_t label = if_label + ++label_id;
    unit_ctx.bytecode->emit(LBL, {label});
    else_if->scope->accept(*this);
    unit_ctx.bytecode->emit(LJMP, {escape_label});
  }

  unit_ctx.bytecode->emit(LBL, {escape_label});

  if (if_node.else_node) {
    if_node.else_node->accept(*this);
  }
}

void StmtNodeVisitor::visit(WhileStmtNode& while_node) {
  /*

  0000 label          0
  0001 jumplabelifnot 0, 1
  ...
  0002 jumplabel      0
  0003 label          1

  */

  repeat_label = unit_ctx.internal.label_count++;
  escape_label = unit_ctx.internal.label_count++;

  operand_t cond_reg = allocator.allocate_register();
  operand_t l_repeat_label = repeat_label.value();
  operand_t l_escape_label = escape_label.value();

  unit_ctx.bytecode->emit(LBL, {l_repeat_label});
  while_node.condition->accept(expression_visitor, cond_reg);
  unit_ctx.bytecode->emit(LJMPIFN, {cond_reg, l_escape_label});
  while_node.body->accept(*this);
  unit_ctx.bytecode->emit(LJMP, {l_repeat_label});
  unit_ctx.bytecode->emit(LBL, {l_escape_label});
  allocator.free_register(cond_reg);

  repeat_label = std::nullopt;
  escape_label = std::nullopt;
}

void StmtNodeVisitor::visit(DeferStmtNode& defer_stmt) {
  unit_ctx.internal.defered_stmts.push_back(defer_stmt.stmt);
}

void StmtNodeVisitor::visit(ExprStmtNode& expr_stmt) {
  ExprNodeBase*& expr = expr_stmt.expression;
  expr->accept(expression_visitor, 0);

  if (CallExprNode* call_node = get_derived_instance<ExprNodeBase, CallExprNode>(expr)) {
    TypeNodeBase* callee_ty = call_node->callee->infer_type(unit_ctx);
    TypeNodeBase* ret_ty = call_node->infer_type(unit_ctx);

    VIA_TINFERENCE_FAILURE(callee_ty, expr);
    VIA_TINFERENCE_FAILURE(ret_ty, expr);

    if (PrimTypeNode* prim_ty = get_derived_instance<TypeNodeBase, PrimTypeNode>(ret_ty)) {
      if (prim_ty->type != IValueType::nil) {
        goto return_value_ignored;
      }
    }
    else {
    return_value_ignored:
      compiler_warning(call_node->begin, call_node->end, "Function return value ignored");
      compiler_info(std::format("Function returns non-nil type '{}'", ret_ty->to_output_string()));
      compiler_output_end();
    }

    // Edit last instruction to drop the return value rather than popping it. This is because if a
    // function call is present under an expression statement, then the result is guaranteed to be
    // ignored.
    Bytecode& last = unit_ctx.bytecode->back();
    last.instruct.op = DROP;
    // Reset operand values to eliminate deceptive values
    last.instruct.operand0 = VIA_OPERAND_INVALID;
    last.instruct.operand1 = VIA_OPERAND_INVALID;
    last.instruct.operand2 = VIA_OPERAND_INVALID;
  }
  else {
    if (unused_expr_handler.has_value()) {
      unused_expression_handler_t handler = *unused_expr_handler;
      handler(expr_stmt);
    }
    else {
      compiler_warning(expr->begin, expr->end, "Expression result unused");
      compiler_output_end();
    }
  }
}

} // namespace via
