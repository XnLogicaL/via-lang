// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "ast.h"
#include "compiler-types.h"
#include "visitor.h"
#include "stack.h"
#include "format-vector.h"

#define depth_tab_space std::string(depth, ' ')

namespace via {

using enum value_type;

std::string modifiers::to_string() const {
  return std::format("{}", is_const ? "const" : "");
}

// ===============================
// lit_expr_node
std::string lit_expr_node::to_string(uint32_t&) {
  if (int* integer_value = std::get_if<int>(&value)) {
    return std::format("Literal<{}>", *integer_value);
  }
  else if (float* floating_point_value = std::get_if<float>(&value)) {
    return std::format("Literal<{}>", *floating_point_value);
  }
  else if (bool* boolean_value = std::get_if<bool>(&value)) {
    return std::format("Literal<{}>", *boolean_value ? "true" : "false");
  }
  else if (std::string* string_value = std::get_if<std::string>(&value)) {
    return std::format("Literal<'{}'>", *string_value);
  }
  else {
    return "Literal<nil>";
  }
}

void lit_expr_node::accept(node_visitor_base& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

p_expr_node_t lit_expr_node::clone() {
  return std::make_unique<lit_expr_node>(*this);
}

p_type_node_t lit_expr_node::infer_type(trans_unit_context&) {
  value_type value_type = std::visit(
    [](auto&& value) {
      using T = std::decay_t<decltype(value)>;
      return data_type<T>::value_type;
    },
    value
  );

  return std::make_unique<primitive_type_node>(value_token, value_type);
}

// ===============================
// sym_expr_node
std::string sym_expr_node::to_string(uint32_t&) {
  return std::format("Symbol<{}>", identifier.lexeme);
}

void sym_expr_node::accept(node_visitor_base& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

p_expr_node_t sym_expr_node::clone() {
  return std::make_unique<sym_expr_node>(*this);
}

p_type_node_t sym_expr_node::infer_type(trans_unit_context& unit_ctx) {
  auto stk_id = unit_ctx.internal.stack->find_symbol(identifier.lexeme);
  if (stk_id.has_value()) {
    auto stk_obj = unit_ctx.internal.stack->at(stk_id.value());
    if (stk_obj.has_value()) {
      return stk_obj->type->clone();
    }

    return nullptr;
  }

  auto global = unit_ctx.internal.globals->get_global(identifier.lexeme);
  if (global.has_value()) {
    return global->type->clone();
  }

  return nullptr;
}

// ===============================
// unary_expr_node
std::string unary_expr_node::to_string(uint32_t& depth) {
  return std::format("Unary<{}>", expression->to_string(depth));
}

void unary_expr_node::accept(node_visitor_base& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

p_expr_node_t unary_expr_node::clone() {
  return std::make_unique<unary_expr_node>(expression->clone());
}

p_type_node_t unary_expr_node::infer_type(trans_unit_context& unit_ctx) {
  p_type_node_t inner = expression->infer_type(unit_ctx);
  if (!inner.get()) {
    return nullptr;
  }

  return is_arithmetic(inner) ? std::move(inner) : nullptr;
}

// ===============================
// grp_expr_node
std::string grp_expr_node::to_string(uint32_t& depth) {
  return std::format("Group<{}>", expression->to_string(depth));
}

void grp_expr_node::accept(node_visitor_base& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

p_expr_node_t grp_expr_node::clone() {
  return std::make_unique<grp_expr_node>(expression->clone());
}

int grp_expr_node::precedence() const {
  return std::numeric_limits<int>::max();
}

p_type_node_t grp_expr_node::infer_type(trans_unit_context& unit_ctx) {
  return expression->infer_type(unit_ctx);
}

// ===============================
// call_expr_node
std::string call_expr_node::to_string(uint32_t& depth) {
  return std::format(
    "call_expr_node<callee {}, args {}>",
    callee->to_string(depth),
    utils::format_vector<p_expr_node_t>(
      arguments, [&depth](const p_expr_node_t& expr) { return expr->to_string(depth); }
    )
  );
}

void call_expr_node::accept(node_visitor_base& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

p_expr_node_t call_expr_node::clone() {
  argument_vector arguments_clone;
  for (p_expr_node_t& argument : arguments) {
    arguments_clone.emplace_back(argument->clone());
  }

  return std::make_unique<call_expr_node>(callee->clone(), std::move(arguments_clone));
}

p_type_node_t call_expr_node::infer_type(trans_unit_context& unit_ctx) {
  if (sym_expr_node* symbol = get_derived_instance<expr_node_base, sym_expr_node>(*callee)) {
    p_type_node_t ty = symbol->infer_type(unit_ctx);
    if (FunctionTypeNode* fn_ty = get_derived_instance<type_node_base, FunctionTypeNode>(*ty)) {
      return fn_ty->returns->clone();
    }
  }

  return nullptr;
}

// ===============================
// index_expr_node
std::string index_expr_node::to_string(uint32_t& depth) {
  return std::format(
    "index_expr_node<object {}, index {}>", object->to_string(depth), index->to_string(depth)
  );
}

void index_expr_node::accept(node_visitor_base& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

p_expr_node_t index_expr_node::clone() {
  return std::make_unique<index_expr_node>(object->clone(), index->clone());
}

p_type_node_t index_expr_node::infer_type(trans_unit_context& unit_ctx) {
  if (sym_expr_node* symbol = get_derived_instance<expr_node_base, sym_expr_node>(*object)) {
    auto stk_id = unit_ctx.internal.stack->find_symbol(symbol->identifier.lexeme);
    if (!stk_id.has_value()) {
      return nullptr;
    }

    auto stk_obj = unit_ctx.internal.stack->at(stk_id.value());
    if (!stk_obj.has_value()) {
      return nullptr;
    }

    return stk_obj->type->clone();
  }

  return nullptr;
}

// ===============================
// bin_expr_node
std::string bin_expr_node::to_string(uint32_t& depth) {
  return std::format(
    "Binary<{} {} {}>",
    lhs_expression->to_string(depth),
    op.lexeme,
    rhs_expression->to_string(depth)
  );
}

void bin_expr_node::accept(node_visitor_base& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

p_expr_node_t bin_expr_node::clone() {
  return std::make_unique<bin_expr_node>(op, lhs_expression->clone(), rhs_expression->clone());
}

p_type_node_t bin_expr_node::infer_type(trans_unit_context& unit_ctx) {
  p_type_node_t lhs = lhs_expression->infer_type(unit_ctx);
  p_type_node_t rhs = rhs_expression->infer_type(unit_ctx);

  if (!lhs || !rhs || !is_integral(lhs) || !is_integral(rhs)) {
    return nullptr;
  }

  if (primitive_type_node* lhs_primitive =
        get_derived_instance<type_node_base, primitive_type_node>(*lhs)) {
    if (primitive_type_node* rhs_primitive =
          get_derived_instance<type_node_base, primitive_type_node>(*rhs)) {
      // If either operand is a floating point, result is floating point
      if (lhs_primitive->type == floating_point || rhs_primitive->type == floating_point) {
        return std::make_unique<primitive_type_node>(lhs_primitive->identifier, floating_point);
      }
      // If both operands are integers, result is integer
      else if (lhs_primitive->type == integer && rhs_primitive->type == integer) {
        return std::make_unique<primitive_type_node>(lhs_primitive->identifier, integer);
      }
    }
  }

  return nullptr;
}

// ===============================
// cast_expr_node
std::string cast_expr_node::to_string(uint32_t& depth) {
  return std::format(
    "cast_expr_node<{} as {}>", expression->to_string(depth), type->to_string(depth)
  );
}

void cast_expr_node::accept(node_visitor_base& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

p_expr_node_t cast_expr_node::clone() {
  return std::make_unique<cast_expr_node>(expression->clone(), type->clone());
}

p_type_node_t cast_expr_node::infer_type(trans_unit_context& unit_ctx) {
  p_type_node_t expr_type = expression->infer_type(unit_ctx);
  if (!is_castable(expr_type, type)) {
    return nullptr;
  }

  return type->clone();
}

// ===============================
// auto_type_node
std::string auto_type_node::to_string(uint32_t&) {
  return "auto_type_node<>";
}

std::string auto_type_node::to_string_x() {
  return "<auto>";
}

void auto_type_node::decay(node_visitor_base& visitor, p_type_node_t& self) {
  self = visitor.visit(*this);
}

p_type_node_t auto_type_node::clone() {
  return std::make_unique<auto_type_node>(*this);
}

// ===============================
// primitive_type_node
std::string primitive_type_node::to_string(uint32_t&) {
  return std::format("primitive_type_node<{}>", magic_enum::enum_name(type));
}

std::string primitive_type_node::to_string_x() {
  return std::string(magic_enum::enum_name(type));
}

p_type_node_t primitive_type_node::clone() {
  return std::make_unique<primitive_type_node>(*this);
}

// ===============================
// generic_type_node
std::string generic_type_node::to_string(uint32_t& depth) {
  return std::format(
    "generic_type_node<{}, {}>",
    identifier.lexeme,
    utils::format_vector<p_type_node_t>(
      generics, [&depth](const p_type_node_t& elem) { return elem->to_string(depth); }
    )
  );
}

std::string generic_type_node::to_string_x() {
  return "<generic-truncated>";
}

void generic_type_node::decay(node_visitor_base& visitor, p_type_node_t& self) {
  self = visitor.visit(*this);
}

p_type_node_t generic_type_node::clone() {
  Generics generics_clone;
  for (p_type_node_t& generic : generics) {
    generics_clone.emplace_back(generic->clone());
  }

  return std::make_unique<generic_type_node>(identifier, std::move(generics_clone), modifiers);
}

// ===============================
// union_type_node
std::string union_type_node::to_string(uint32_t& depth) {
  return std::format("union_type_node<{} & {}>", lhs->to_string(depth), rhs->to_string(depth));
}

std::string union_type_node::to_string_x() {
  return "<union-truncated>";
}

void union_type_node::decay(node_visitor_base& visitor, p_type_node_t& self) {
  self = visitor.visit(*this);
}

p_type_node_t union_type_node::clone() {
  return std::make_unique<union_type_node>(lhs->clone(), rhs->clone());
}

// ===============================
// func_stmt_node
std::string FunctionTypeNode::to_string(uint32_t& depth) {
  return std::format(
    "FunctionTypeNode<{} -> {}>",
    utils::format_vector<p_type_node_t>(
      parameters, [&depth](const p_type_node_t& elem) { return elem->to_string(depth); }
    ),
    returns->to_string(depth)
  );
}

std::string FunctionTypeNode::to_string_x() {
  return "<function-truncated>";
}

void FunctionTypeNode::decay(node_visitor_base& visitor, p_type_node_t& self) {
  self = visitor.visit(*this);
}

p_type_node_t FunctionTypeNode::clone() {
  parameter_vector parameters_clone;
  for (p_type_node_t& parameter : parameters) {
    parameters_clone.emplace_back(parameter->clone());
  }

  return std::make_unique<FunctionTypeNode>(std::move(parameters_clone), returns->clone());
}

// ===============================
// decl_stmt_node
std::string decl_stmt_node::to_string(uint32_t& depth) {
  return std::format(
    "{}Declaration<{} {} {}: {} = {}>",
    DEPTH_TAB_SPACE,
    is_global ? "global" : "local",
    modifiers.to_string(),
    identifier.lexeme,
    type->to_string(depth),
    value_expression->to_string(depth)
  );
}

void decl_stmt_node::accept(node_visitor_base& visitor) {
  visitor.visit(*this);
}

p_stmt_node_t decl_stmt_node::clone() {
  return std::make_unique<decl_stmt_node>(
    is_global, modifiers, identifier, value_expression->clone(), type->clone()
  );
}

// ===============================
// scope_stmt_node
std::string scope_stmt_node::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << DEPTH_TAB_SPACE << "Scope<>\n";

  depth++;

  for (const p_stmt_node_t& pstmt : statements) {
    oss << pstmt->to_string(depth) << "\n";
  }

  depth--;

  oss << DEPTH_TAB_SPACE << "EndScope<>";
  return oss.str();
}

void scope_stmt_node::accept(node_visitor_base& visitor) {
  visitor.visit(*this);
}

p_stmt_node_t scope_stmt_node::clone() {
  Statements statements_clone;
  for (p_stmt_node_t& statement : statements) {
    statements_clone.emplace_back(statement->clone());
  }

  return std::make_unique<scope_stmt_node>(std::move(statements_clone));
}

// ===============================
// func_stmt_node
std::string func_stmt_node::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << DEPTH_TAB_SPACE
      << std::format(
           "Function<{} {} {}>\n",
           is_global ? "global" : "local",
           modifiers.to_string(),
           identifier.lexeme
         );

  depth++;

  for (const param_node& parameter : parameters) {
    oss << DEPTH_TAB_SPACE << std::format("Parameter<{}>", parameter.identifier.lexeme) << "\n";
  }

  for (const p_stmt_node_t& stmt : dynamic_cast<scope_stmt_node&>(*body).statements) {
    oss << stmt->to_string(depth) << "\n";
  }

  depth--;

  oss << DEPTH_TAB_SPACE << "EndFunction<>";
  return oss.str();
}

void func_stmt_node::accept(node_visitor_base& visitor) {
  visitor.visit(*this);
}

p_stmt_node_t func_stmt_node::clone() {
  parameters_t parameters_clone;
  for (param_node& parameter : parameters) {
    parameters_clone.emplace_back(
      parameter.identifier, parameter.modifiers, parameter.type->clone()
    );
  }

  return std::make_unique<func_stmt_node>(
    is_global, modifiers, identifier, body->clone(), returns->clone(), std::move(parameters_clone)
  );
}

// ===============================
// assign_stmt_node
std::string assign_stmt_node::to_string(uint32_t& depth) {
  return std::format(
    "{}Assign<{} {}= {}>",
    DEPTH_TAB_SPACE,
    augmentation_operator.lexeme,
    assignee->to_string(depth),
    value->to_string(depth)
  );
}

void assign_stmt_node::accept(node_visitor_base& visitor) {
  visitor.visit(*this);
}

p_stmt_node_t assign_stmt_node::clone() {
  return std::make_unique<assign_stmt_node>(
    assignee->clone(), augmentation_operator, value->clone()
  );
}

// ===============================
// func_stmt_node
std::string if_stmt_node::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << DEPTH_TAB_SPACE << std::format("if_stmt_node<{}>", condition->to_string(depth)) << "\n";

  depth++;

  oss << scope->to_string(depth) << "\n";

  for (const elseif_node& elseif_node : elseif_nodes) {
    oss << DEPTH_TAB_SPACE << std::format("ElseIf<{}>", elseif_node.condition->to_string(depth))
        << "\n";

    depth++;

    oss << elseif_node.scope->to_string(depth) << "\n";

    depth--;
    oss << DEPTH_TAB_SPACE << "EndElseIf<>\n";
  }

  if (else_node.get()) {
    oss << DEPTH_TAB_SPACE << "Else<>"
        << "\n";
    depth++;
    oss << else_node->to_string(depth) << "\n";
    depth--;
    oss << DEPTH_TAB_SPACE << "EndElse<>"
        << "\n";
  }

  depth--;

  oss << DEPTH_TAB_SPACE << "EndIf<>";
  return oss.str();
}

void if_stmt_node::accept(node_visitor_base& visitor) {
  visitor.visit(*this);
}

p_stmt_node_t if_stmt_node::clone() {
  elseif_nodes_t elseif_nodes_clone;
  for (elseif_node& else_if : elseif_nodes) {
    elseif_nodes_clone.emplace_back(else_if.condition->clone(), else_if.scope->clone());
  }

  return std::make_unique<if_stmt_node>(
    condition->clone(), scope->clone(), else_node->clone(), std::move(elseif_nodes_clone)
  );
}

// ===============================
// return_stmt_node

std::string return_stmt_node::to_string(uint32_t& depth) {
  return DEPTH_TAB_SPACE + std::format("return_stmt_node<{}>", expression->to_string(depth));
}

p_stmt_node_t return_stmt_node::clone() {
  return std::make_unique<return_stmt_node>(expression->clone());
}

void return_stmt_node::accept(node_visitor_base& visitor) {
  visitor.visit(*this);
}

// ===============================
// break_stmt_node

std::string break_stmt_node::to_string(uint32_t& depth) {
  return DEPTH_TAB_SPACE + "break_stmt_node<>";
}

p_stmt_node_t break_stmt_node::clone() {
  return std::make_unique<break_stmt_node>(token);
}

void break_stmt_node::accept(node_visitor_base& visitor) {
  visitor.visit(*this);
}

// ===============================
// continue_stmt_node

std::string continue_stmt_node::to_string(uint32_t& depth) {
  return DEPTH_TAB_SPACE + "continue_stmt_node<>";
}

p_stmt_node_t continue_stmt_node::clone() {
  return std::make_unique<continue_stmt_node>(token);
}

void continue_stmt_node::accept(node_visitor_base& visitor) {
  visitor.visit(*this);
}

// ===============================
// while_stmt_node
std::string while_stmt_node::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << DEPTH_TAB_SPACE << std::format("while_stmt_node<{}>", condition->to_string(depth)) << "\n";

  depth++;

  for (const p_stmt_node_t& stmt : dynamic_cast<scope_stmt_node&>(*body).statements) {
    oss << stmt->to_string(depth) << "\n";
  }

  depth--;

  oss << DEPTH_TAB_SPACE << "EndWhile<>";
  return oss.str();
}

void while_stmt_node::accept(node_visitor_base& visitor) {
  visitor.visit(*this);
}

p_stmt_node_t while_stmt_node::clone() {
  return std::make_unique<while_stmt_node>(condition->clone(), body->clone());
}

// ===============================
// expr_stmt_node
std::string expr_stmt_node::to_string(uint32_t& depth) {
  return std::format("{}expr_stmt_node<{}>", DEPTH_TAB_SPACE, expression->to_string(depth));
}

void expr_stmt_node::accept(node_visitor_base& visitor) {
  visitor.visit(*this);
}

p_stmt_node_t expr_stmt_node::clone() {
  return std::make_unique<expr_stmt_node>(expression->clone());
}

} // namespace via
