// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "ast.h"
#include "compiler-types.h"
#include "visitor.h"
#include "stack.h"
#include "format-vector.h"

#define DEPTH_TAB_SPACE std::string(depth, ' ')

VIA_NAMESPACE_BEGIN

using enum ValueType;

std::string Modifiers::to_string() const {
  return std::format("{}", is_const ? "const" : "");
}

// ===============================
// LiteralExprNode
std::string LiteralExprNode::to_string(uint32_t&) {
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

void LiteralExprNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode LiteralExprNode::clone() {
  return std::make_unique<LiteralExprNode>(*this);
}

pTypeNode LiteralExprNode::infer_type(TransUnitContext&) {
  ValueType value_type = std::visit(
    [](auto&& value) {
      using T = std::decay_t<decltype(value)>;
      return DataType<T>::value_type;
    },
    value
  );

  return std::make_unique<PrimitiveTypeNode>(value_token, value_type);
}

// ===============================
// SymbolExprNode
std::string SymbolExprNode::to_string(uint32_t&) {
  return std::format("Symbol<{}>", identifier.lexeme);
}

void SymbolExprNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode SymbolExprNode::clone() {
  return std::make_unique<SymbolExprNode>(*this);
}

pTypeNode SymbolExprNode::infer_type(TransUnitContext& unit_ctx) {
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
// UnaryExprNode
std::string UnaryExprNode::to_string(uint32_t& depth) {
  return std::format("Unary<{}>", expression->to_string(depth));
}

void UnaryExprNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode UnaryExprNode::clone() {
  return std::make_unique<UnaryExprNode>(expression->clone());
}

pTypeNode UnaryExprNode::infer_type(TransUnitContext& unit_ctx) {
  pTypeNode inner = expression->infer_type(unit_ctx);
  if (!inner.get()) {
    return nullptr;
  }

  return is_arithmetic(inner) ? std::move(inner) : nullptr;
}

// ===============================
// GroupExprNode
std::string GroupExprNode::to_string(uint32_t& depth) {
  return std::format("Group<{}>", expression->to_string(depth));
}

void GroupExprNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode GroupExprNode::clone() {
  return std::make_unique<GroupExprNode>(expression->clone());
}

int GroupExprNode::precedence() const {
  return std::numeric_limits<int>::max();
}

pTypeNode GroupExprNode::infer_type(TransUnitContext& unit_ctx) {
  return expression->infer_type(unit_ctx);
}

// ===============================
// CallExprNode
std::string CallExprNode::to_string(uint32_t& depth) {
  return std::format(
    "CallExprNode<callee {}, args {}>",
    callee->to_string(depth),
    utils::format_vector<pExprNode>(
      arguments, [&depth](const pExprNode& expr) { return expr->to_string(depth); }
    )
  );
}

void CallExprNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode CallExprNode::clone() {
  argument_vector arguments_clone;
  for (pExprNode& argument : arguments) {
    arguments_clone.emplace_back(argument->clone());
  }

  return std::make_unique<CallExprNode>(callee->clone(), std::move(arguments_clone));
}

pTypeNode CallExprNode::infer_type(TransUnitContext& unit_ctx) {
  if (SymbolExprNode* symbol = get_derived_instance<ExprNode, SymbolExprNode>(*callee)) {
    pTypeNode ty = symbol->infer_type(unit_ctx);
    if (FunctionTypeNode* fn_ty = get_derived_instance<TypeNode, FunctionTypeNode>(*ty)) {
      return fn_ty->returns->clone();
    }
  }

  return nullptr;
}

// ===============================
// IndexExprNode
std::string IndexExprNode::to_string(uint32_t& depth) {
  return std::format(
    "IndexExprNode<object {}, index {}>", object->to_string(depth), index->to_string(depth)
  );
}

void IndexExprNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode IndexExprNode::clone() {
  return std::make_unique<IndexExprNode>(object->clone(), index->clone());
}

pTypeNode IndexExprNode::infer_type(TransUnitContext& unit_ctx) {
  if (SymbolExprNode* symbol = get_derived_instance<ExprNode, SymbolExprNode>(*object)) {
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
// BinaryExprNode
std::string BinaryExprNode::to_string(uint32_t& depth) {
  return std::format(
    "Binary<{} {} {}>",
    lhs_expression->to_string(depth),
    op.lexeme,
    rhs_expression->to_string(depth)
  );
}

void BinaryExprNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode BinaryExprNode::clone() {
  return std::make_unique<BinaryExprNode>(op, lhs_expression->clone(), rhs_expression->clone());
}

pTypeNode BinaryExprNode::infer_type(TransUnitContext& unit_ctx) {
  pTypeNode lhs = lhs_expression->infer_type(unit_ctx);
  pTypeNode rhs = rhs_expression->infer_type(unit_ctx);

  if (!lhs || !rhs || !is_integral(lhs) || !is_integral(rhs)) {
    return nullptr;
  }

  if (PrimitiveTypeNode* lhs_primitive = get_derived_instance<TypeNode, PrimitiveTypeNode>(*lhs)) {
    if (PrimitiveTypeNode* rhs_primitive =
          get_derived_instance<TypeNode, PrimitiveTypeNode>(*rhs)) {
      // If either operand is a floating point, result is floating point
      if (lhs_primitive->type == floating_point || rhs_primitive->type == floating_point) {
        return std::make_unique<PrimitiveTypeNode>(lhs_primitive->identifier, floating_point);
      }
      // If both operands are integers, result is integer
      else if (lhs_primitive->type == integer && rhs_primitive->type == integer) {
        return std::make_unique<PrimitiveTypeNode>(lhs_primitive->identifier, integer);
      }
    }
  }

  return nullptr;
}

// ===============================
// TypeCastExprNode
std::string TypeCastExprNode::to_string(uint32_t& depth) {
  return std::format(
    "TypeCastExprNode<{} as {}>", expression->to_string(depth), type->to_string(depth)
  );
}

void TypeCastExprNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode TypeCastExprNode::clone() {
  return std::make_unique<TypeCastExprNode>(expression->clone(), type->clone());
}

pTypeNode TypeCastExprNode::infer_type(TransUnitContext& unit_ctx) {
  pTypeNode expr_type = expression->infer_type(unit_ctx);
  if (!is_castable(expr_type, type)) {
    return nullptr;
  }

  return type->clone();
}

// ===============================
// AutoTypeNode
std::string AutoTypeNode::to_string(uint32_t&) {
  return "AutoTypeNode<>";
}

std::string AutoTypeNode::to_string_x() {
  return "<auto>";
}

void AutoTypeNode::decay(NodeVisitor& visitor, pTypeNode& self) {
  self = visitor.visit(*this);
}

pTypeNode AutoTypeNode::clone() {
  return std::make_unique<AutoTypeNode>(*this);
}

// ===============================
// PrimitiveTypeNode
std::string PrimitiveTypeNode::to_string(uint32_t&) {
  return std::format("PrimitiveTypeNode<{}>", magic_enum::enum_name(type));
}

std::string PrimitiveTypeNode::to_string_x() {
  return std::string(magic_enum::enum_name(type));
}

pTypeNode PrimitiveTypeNode::clone() {
  return std::make_unique<PrimitiveTypeNode>(*this);
}

// ===============================
// GenericTypeNode
std::string GenericTypeNode::to_string(uint32_t& depth) {
  return std::format(
    "GenericTypeNode<{}, {}>",
    identifier.lexeme,
    utils::format_vector<pTypeNode>(
      generics, [&depth](const pTypeNode& elem) { return elem->to_string(depth); }
    )
  );
}

std::string GenericTypeNode::to_string_x() {
  return "<generic-truncated>";
}

void GenericTypeNode::decay(NodeVisitor& visitor, pTypeNode& self) {
  self = visitor.visit(*this);
}

pTypeNode GenericTypeNode::clone() {
  Generics generics_clone;
  for (pTypeNode& generic : generics) {
    generics_clone.emplace_back(generic->clone());
  }

  return std::make_unique<GenericTypeNode>(identifier, std::move(generics_clone), modifiers);
}

// ===============================
// UnionTypeNode
std::string UnionTypeNode::to_string(uint32_t& depth) {
  return std::format("UnionTypeNode<{} & {}>", lhs->to_string(depth), rhs->to_string(depth));
}

std::string UnionTypeNode::to_string_x() {
  return "<union-truncated>";
}

void UnionTypeNode::decay(NodeVisitor& visitor, pTypeNode& self) {
  self = visitor.visit(*this);
}

pTypeNode UnionTypeNode::clone() {
  return std::make_unique<UnionTypeNode>(lhs->clone(), rhs->clone());
}

// ===============================
// FunctionStmtNode
std::string FunctionTypeNode::to_string(uint32_t& depth) {
  return std::format(
    "FunctionTypeNode<{} -> {}>",
    utils::format_vector<pTypeNode>(
      parameters, [&depth](const pTypeNode& elem) { return elem->to_string(depth); }
    ),
    returns->to_string(depth)
  );
}

std::string FunctionTypeNode::to_string_x() {
  return "<function-truncated>";
}

void FunctionTypeNode::decay(NodeVisitor& visitor, pTypeNode& self) {
  self = visitor.visit(*this);
}

pTypeNode FunctionTypeNode::clone() {
  parameter_vector parameters_clone;
  for (pTypeNode& parameter : parameters) {
    parameters_clone.emplace_back(parameter->clone());
  }

  return std::make_unique<FunctionTypeNode>(std::move(parameters_clone), returns->clone());
}

// ===============================
// DeclarationStmtNode
std::string DeclarationStmtNode::to_string(uint32_t& depth) {
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

void DeclarationStmtNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode DeclarationStmtNode::clone() {
  return std::make_unique<DeclarationStmtNode>(
    is_global, modifiers, identifier, value_expression->clone(), type->clone()
  );
}

// ===============================
// ScopeStmtNode
std::string ScopeStmtNode::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << DEPTH_TAB_SPACE << "Scope<>\n";

  depth++;

  for (const pStmtNode& pstmt : statements) {
    oss << pstmt->to_string(depth) << "\n";
  }

  depth--;

  oss << DEPTH_TAB_SPACE << "EndScope<>";
  return oss.str();
}

void ScopeStmtNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode ScopeStmtNode::clone() {
  Statements statements_clone;
  for (pStmtNode& statement : statements) {
    statements_clone.emplace_back(statement->clone());
  }

  return std::make_unique<ScopeStmtNode>(std::move(statements_clone));
}

// ===============================
// FunctionStmtNode
std::string FunctionStmtNode::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << DEPTH_TAB_SPACE
      << std::format(
           "Function<{} {} {}>\n",
           is_global ? "global" : "local",
           modifiers.to_string(),
           identifier.lexeme
         );

  depth++;

  for (const ParameterNode& parameter : parameters) {
    oss << DEPTH_TAB_SPACE << std::format("Parameter<{}>", parameter.identifier.lexeme) << "\n";
  }

  for (const pStmtNode& stmt : dynamic_cast<ScopeStmtNode&>(*body).statements) {
    oss << stmt->to_string(depth) << "\n";
  }

  depth--;

  oss << DEPTH_TAB_SPACE << "EndFunction<>";
  return oss.str();
}

void FunctionStmtNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode FunctionStmtNode::clone() {
  Parameters parameters_clone;
  for (ParameterNode& parameter : parameters) {
    parameters_clone.emplace_back(
      parameter.identifier, parameter.modifiers, parameter.type->clone()
    );
  }

  return std::make_unique<FunctionStmtNode>(
    is_global, modifiers, identifier, body->clone(), returns->clone(), std::move(parameters_clone)
  );
}

// ===============================
// AssignStmtNode
std::string AssignStmtNode::to_string(uint32_t& depth) {
  return std::format(
    "{}Assign<{} {}= {}>",
    DEPTH_TAB_SPACE,
    augmentation_operator.lexeme,
    assignee->to_string(depth),
    value->to_string(depth)
  );
}

void AssignStmtNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode AssignStmtNode::clone() {
  return std::make_unique<AssignStmtNode>(assignee->clone(), augmentation_operator, value->clone());
}

// ===============================
// FunctionStmtNode
std::string IfStmtNode::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << DEPTH_TAB_SPACE << std::format("IfStmtNode<{}>", condition->to_string(depth)) << "\n";

  depth++;

  oss << scope->to_string(depth) << "\n";

  for (const ElseIfNode& elseif_node : elseif_nodes) {
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

void IfStmtNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode IfStmtNode::clone() {
  ElseIfNodes elseif_nodes_clone;
  for (ElseIfNode& else_if : elseif_nodes) {
    elseif_nodes_clone.emplace_back(else_if.condition->clone(), else_if.scope->clone());
  }

  return std::make_unique<IfStmtNode>(
    condition->clone(), scope->clone(), else_node->clone(), std::move(elseif_nodes_clone)
  );
}

// ===============================
// ReturnStmtNode

std::string ReturnStmtNode::to_string(uint32_t& depth) {
  return DEPTH_TAB_SPACE + std::format("ReturnStmtNode<{}>", expression->to_string(depth));
}

pStmtNode ReturnStmtNode::clone() {
  return std::make_unique<ReturnStmtNode>(expression->clone());
}

void ReturnStmtNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

// ===============================
// BreakStmtNode

std::string BreakStmtNode::to_string(uint32_t& depth) {
  return DEPTH_TAB_SPACE + "BreakStmtNode<>";
}

pStmtNode BreakStmtNode::clone() {
  return std::make_unique<BreakStmtNode>(token);
}

void BreakStmtNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

// ===============================
// ContinueStmtNode

std::string ContinueStmtNode::to_string(uint32_t& depth) {
  return DEPTH_TAB_SPACE + "ContinueStmtNode<>";
}

pStmtNode ContinueStmtNode::clone() {
  return std::make_unique<ContinueStmtNode>(token);
}

void ContinueStmtNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

// ===============================
// WhileStmtNode
std::string WhileStmtNode::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << DEPTH_TAB_SPACE << std::format("WhileStmtNode<{}>", condition->to_string(depth)) << "\n";

  depth++;

  for (const pStmtNode& stmt : dynamic_cast<ScopeStmtNode&>(*body).statements) {
    oss << stmt->to_string(depth) << "\n";
  }

  depth--;

  oss << DEPTH_TAB_SPACE << "EndWhile<>";
  return oss.str();
}

void WhileStmtNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode WhileStmtNode::clone() {
  return std::make_unique<WhileStmtNode>(condition->clone(), body->clone());
}

// ===============================
// ExprStmtNode
std::string ExprStmtNode::to_string(uint32_t& depth) {
  return std::format("{}ExprStmtNode<{}>", DEPTH_TAB_SPACE, expression->to_string(depth));
}

void ExprStmtNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode ExprStmtNode::clone() {
  return std::make_unique<ExprStmtNode>(expression->clone());
}

VIA_NAMESPACE_END
