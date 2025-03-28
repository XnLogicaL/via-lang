// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "bit-utility.h"
#include "string-utility.h"
#include "compiler-types.h"
#include "visitor.h"
#include "stack.h"
#include "format-vector.h"

namespace via {

using enum opcode;

void stmt_node_visitor::visit(decl_stmt_node& declaration_node) {
  bool is_global = declaration_node.is_global;
  bool is_const = declaration_node.modifs.is_const;

  expr_node_base& val = *declaration_node.value_expression;
  p_type_node_t val_ty = val.infer_type(unit_ctx);
  token ident = declaration_node.identifier;

  std::string symbol = ident.lexeme;

  if (is_global) {
    std::string comment = symbol;
    std::optional<global_obj> previously_declared = unit_ctx.internal.globals->get_global(symbol);

    if (previously_declared.has_value()) {
      compiler_error(ident, std::format("Attempt to re-declare global '{}'", symbol));
      compiler_info(previously_declared->token, "Previously declared here");
    }
    else {
      operand_t value_reg = allocator.allocate_register();
      operand_t symbol_hash = hash_string_custom(symbol.c_str());

      global_obj global{.token = ident, .symbol = symbol, .type = std::move(val_ty)};
      unit_ctx.internal.globals->declare_global(std::move(global));
      declaration_node.value_expression->accept(expression_visitor, value_reg);
      unit_ctx.bytecode->emit(SETGLOBAL, {value_reg, symbol_hash}, comment);
      allocator.free_register(value_reg);
    }
  }
  else {
    std::string comment = std::format("{}", symbol);

    if (is_constant_expression(val)) {
      lit_expr_node& literal = dynamic_cast<lit_expr_node&>(val);

      // Check for nil
      if (std::get_if<std::monostate>(&literal.value)) {
        unit_ctx.bytecode->emit(PUSHNIL, {}, comment);
        unit_ctx.internal.stack->push({
          .is_const = is_const,
          .is_constexpr = true,
          .symbol = symbol,
          .type = std::make_unique<primitive_type_node>(literal.value_token, value_type::nil),
        });
      }
      // Check for integer
      else if (int* int_value = std::get_if<int>(&literal.value)) {
        uint32_t final_value = *int_value;
        auto operands = reinterpret_u32_as_2u16(final_value);

        unit_ctx.bytecode->emit(PUSHINT, {operands.l, operands.r}, comment);
        unit_ctx.internal.stack->push({
          .is_const = is_const,
          .is_constexpr = true,
          .symbol = symbol,
          .type = std::make_unique<primitive_type_node>(literal.value_token, value_type::integer),
        });
      }
      // Check for float
      else if (float* float_value = std::get_if<float>(&literal.value)) {
        uint32_t final_value = std::bit_cast<uint32_t>(*float_value);
        auto operands = reinterpret_u32_as_2u16(final_value);

        unit_ctx.bytecode->emit(PUSHFLOAT, {operands.l, operands.r}, comment);
        unit_ctx.internal.stack->push({
          .is_const = is_const,
          .is_constexpr = true,
          .symbol = symbol,
          .type =
            std::make_unique<primitive_type_node>(literal.value_token, value_type::floating_point),
        });
      }
      // Check for boolean
      else if (bool* bool_value = std::get_if<bool>(&literal.value)) {
        unit_ctx.bytecode->emit(*bool_value ? PUSHTRUE : PUSHFALSE, {}, comment);
        unit_ctx.internal.stack->push({
          .is_const = is_const,
          .is_constexpr = true,
          .symbol = symbol,
          .type = std::make_unique<primitive_type_node>(literal.value_token, value_type::boolean),
        });
      }
      // Other constant
      else {
        const value_obj& constant = construct_constant(dynamic_cast<lit_expr_node&>(val));
        const operand_t const_id = unit_ctx.constants->push_constant(constant);

        unit_ctx.bytecode->emit(PUSHK, {const_id}, comment);
        unit_ctx.internal.stack->push({
          .is_const = is_const,
          .is_constexpr = true,
          .symbol = symbol,
          .type = std::make_unique<primitive_type_node>(literal.value_token, constant.type),
        });
      }
    }
    else {
      operand_t dst = allocator.allocate_register();

      declaration_node.value_expression->accept(expression_visitor, dst);
      unit_ctx.bytecode->emit(PUSH, {dst}, comment);
      unit_ctx.internal.stack->push({
        .is_const = is_const,
        .is_constexpr = false,
        .symbol = symbol,
        .type = std::make_unique<primitive_type_node>(declaration_node.identifier, value_type::nil),
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

void stmt_node_visitor::visit(scope_stmt_node& scope_node) {
  operand_t stack_pointer = unit_ctx.internal.stack->size();

  for (const p_stmt_node_t& pstmt : scope_node.statements) {
    pstmt->accept(*this);
  }

  operand_t stack_allocations = unit_ctx.internal.stack->size() - stack_pointer;
  for (; stack_allocations > 0; stack_allocations--) {
    unit_ctx.bytecode->emit(DROP);
  }
}

void stmt_node_visitor::visit(func_stmt_node& function_node) {
  using parameters_t = func_stmt_node::parameters_t;

  operand_t function_reg = allocator.allocate_register();
  parameters_t parameters;

  unit_ctx.internal.stack->push({
    .is_const = function_node.modifs.is_const,
    .is_constexpr = false,
    .symbol = function_node.identifier.lexeme,
    .type = std::make_unique<primitive_type_node>(function_node.identifier, value_type::function),
  });

  unit_ctx.internal.stack->function_stack.push(func_stmt_node::stack_node(
    function_node.is_global,
    unit_ctx.internal.stack->size(),
    function_node.modifs,
    function_node.identifier,
    std::move(parameters)
  ));

  for (auto& parameter : function_node.parameters) {
    parameters.emplace_back(parameter.identifier, parameter.modifs, parameter.type->clone());
    parameter.type->decay(decay_visitor, parameter.type);
  }

  function_node.returns->decay(decay_visitor, function_node.returns);
  function_node.accept(type_visitor);
  unit_ctx.bytecode->emit(LOADFUNCTION, {function_reg}, function_node.identifier.lexeme);

  scope_stmt_node& scope = dynamic_cast<scope_stmt_node&>(*function_node.body);
  for (const p_stmt_node_t& pstmt : scope.statements) {
    const stmt_node_base& stmt = *pstmt;

    const decl_stmt_node* declaration_node = dynamic_cast<const decl_stmt_node*>(&stmt);
    const func_stmt_node* function_node = dynamic_cast<const func_stmt_node*>(&stmt);

    if (declaration_node || function_node) {
      bool is_global = declaration_node ? declaration_node->is_global : function_node->is_global;
      token identifier =
        declaration_node ? declaration_node->identifier : function_node->identifier;

      if (is_global) {
        compiler_error(identifier, "Function scopes cannot declare globals");
        compiler_info(
          "Function scopes containing global declarations may cause previously declared "
          "globals to be re-declared, therefore are not allowed."
        );
        break;
      }
    }

    pstmt->accept(*this);
  }

  bytecode last_bytecode = unit_ctx.bytecode->back();
  opcode last_opcode = last_bytecode.instruction.op;

  if (last_opcode != RETURN && last_opcode != RETURNNIL) {
    unit_ctx.bytecode->emit(RETURNNIL);
  }

  token symbol_token = function_node.identifier;
  std::string symbol = symbol_token.lexeme;
  uint32_t symbol_hash = hash_string_custom(symbol.c_str());

  if (function_node.is_global) {
    if (unit_ctx.internal.globals->was_declared(symbol)) {
      compiler_error(symbol_token, std::format("Attempt to re-declare global '{}'", symbol));
      return;
    }

    auto operands = reinterpret_u32_as_2u16(symbol_hash);
    unit_ctx.bytecode->emit(SETGLOBAL, {function_reg, operands.l, operands.r});
  }
  else {
    unit_ctx.bytecode->emit(PUSH, {function_reg});
  }

  allocator.free_register(function_reg);
  unit_ctx.internal.stack->function_stack.pop();
}

void stmt_node_visitor::visit(assign_stmt_node& assign_node) {
  if (sym_expr_node* symbol_node =
        get_derived_instance<expr_node_base, sym_expr_node>(*assign_node.assignee)) {
    token symbol_token = symbol_node->identifier;

    std::string symbol = symbol_token.lexeme;
    std::optional<operand_t> stk_id = unit_ctx.internal.stack->find_symbol(symbol);

    if (stk_id.has_value()) {
      const auto& test_stack_member = unit_ctx.internal.stack->at(stk_id.value());

      if (test_stack_member.has_value() && test_stack_member->is_const) {
        compiler_error(symbol_token, std::format("Assignment to constant variable '{}'", symbol));
        return;
      }

      operand_t value_reg = allocator.allocate_register();
      assign_node.value->accept(expression_visitor, value_reg);

      if (unit_ctx.internal.stack->function_stack.size() > 0) {
        const auto& current_closure = unit_ctx.internal.stack->function_stack.top();
        if (current_closure.upvalues < stk_id.value()) {
          unit_ctx.bytecode->emit(SETSTACK, {value_reg, stk_id.value()}, symbol);
        }
        else {
          unit_ctx.bytecode->emit(SETUPVALUE, {value_reg, stk_id.value()}, symbol);
        }
      }
    }
    else {
      compiler_error(symbol_token, "Assignment to invalid lvalue, symbol has not been declared");
    }
  }
  else {
    compiler_error(
      assign_node.assignee->begin, assign_node.assignee->end, "Assignment to invalid lvalue"
    );
  }
}

void stmt_node_visitor::visit(return_stmt_node& return_node) {
  auto& this_function = unit_ctx.internal.stack->function_stack.top();
  if (return_node.expression.get()) {
    operand_t expr_reg = allocator.allocate_register();

    return_node.expression->accept(expression_visitor, expr_reg);
    unit_ctx.bytecode->emit(RETURN, {expr_reg}, this_function.identifier.lexeme);
    allocator.free_register(expr_reg);
  }
  else {
    unit_ctx.bytecode->emit(RETURNNIL, {}, this_function.identifier.lexeme);
  }
}

void stmt_node_visitor::visit(break_stmt_node& break_node) {
  if (!escape_label.has_value()) {
    compiler_error(break_node.tok, "'break' statement not within loop or switch");
  }
  else {
    unit_ctx.bytecode->emit(JUMPLABEL, {escape_label.value()}, "break");
  }
}

void stmt_node_visitor::visit(continue_stmt_node& continue_node) {
  if (!repeat_label.has_value()) {
    compiler_error(continue_node.tok, "'continue' statement not within loop");
  }
  else {
    unit_ctx.bytecode->emit(JUMPLABEL, {repeat_label.value()}, "continue");
  }
}

void stmt_node_visitor::visit(if_stmt_node& if_node) {
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
  unit_ctx.bytecode->emit(JUMPLABELIF, {cond_reg, if_label}, "if");

  for (const auto& elseif_node : if_node.elseif_nodes) {
    operand_t label = unit_ctx.internal.label_count++;

    elseif_node.condition->accept(expression_visitor, cond_reg);
    unit_ctx.bytecode->emit(
      JUMPLABELIF, {cond_reg, label}, std::format("elseif #{}", label - if_label)
    );
  }

  allocator.free_register(cond_reg);

  operand_t escape_label = unit_ctx.internal.label_count++;

  unit_ctx.bytecode->emit(JUMPLABEL, {escape_label}, "else");
  unit_ctx.bytecode->emit(LABEL, {if_label});
  if_node.scope->accept(*this);
  unit_ctx.bytecode->emit(JUMPLABEL, {escape_label});

  size_t label_id = 0;
  for (const auto& elseif_node : if_node.elseif_nodes) {
    operand_t label = if_label + ++label_id;

    unit_ctx.bytecode->emit(LABEL, {label});
    elseif_node.scope->accept(*this);
    unit_ctx.bytecode->emit(JUMPLABEL, {escape_label});
  }

  unit_ctx.bytecode->emit(LABEL, {escape_label});

  if (if_node.else_node.get()) {
    if_node.else_node->accept(*this);
  }
}

void stmt_node_visitor::visit(while_stmt_node& while_node) {
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

  unit_ctx.bytecode->emit(LABEL, {l_repeat_label});
  while_node.condition->accept(expression_visitor, cond_reg);
  unit_ctx.bytecode->emit(JUMPLABELIFNOT, {cond_reg, l_escape_label});
  while_node.body->accept(*this);
  unit_ctx.bytecode->emit(JUMPLABEL, {l_repeat_label});
  unit_ctx.bytecode->emit(LABEL, {l_escape_label});
  allocator.free_register(cond_reg);

  repeat_label = std::nullopt;
  escape_label = std::nullopt;
}

void stmt_node_visitor::visit(expr_stmt_node& expr_stmt) {
  p_expr_node_t& expr = expr_stmt.expression;
  expr->accept(expression_visitor, 0);

  if (call_expr_node* call_node = get_derived_instance<expr_node_base, call_expr_node>(*expr)) {
    p_type_node_t callee_ty = call_node->callee->infer_type(unit_ctx);
    p_type_node_t ret_ty = call_node->infer_type(unit_ctx);

    vl_tinference_failure(callee_ty, expr);
    vl_tinference_failure(ret_ty, expr);

    if (primitive_type_node* prim_ty =
          get_derived_instance<type_node_base, primitive_type_node>(*ret_ty)) {
      if (prim_ty->type != value_type::nil) {
        goto return_value_ignored;
      }
    }
    else {
    return_value_ignored:
      compiler_warning(call_node->begin, call_node->end, "Function return value ignored");
      compiler_info(std::format("Function returns non-nil type '{}'", ret_ty->to_string_x()));
    }

    // Edit last instruction to drop the return value rather than popping it.
    bytecode& last = unit_ctx.bytecode->back();
    last.instruction.op = DROP;
  }
  else {
    compiler_warning(expr->begin, expr->end, "Expression result unused");
  }
}

} // namespace via
