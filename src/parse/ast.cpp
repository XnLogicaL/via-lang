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
// LiteralNode
std::string LiteralNode::to_string(uint32_t&) {
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

void LiteralNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode LiteralNode::clone() {
  return std::make_unique<LiteralNode>(*this);
}

pTypeNode LiteralNode::infer_type(TransUnitContext&) {
  ValueType value_type = std::visit(
      [](auto&& value) {
        using T = std::decay_t<decltype(value)>;
        return DataType<T>::value_type;
      },
      value
  );

  return std::make_unique<PrimitiveNode>(value_token, value_type);
}

// ===============================
// SymbolNode
std::string SymbolNode::to_string(uint32_t&) {
  return std::format("Symbol<{}>", identifier.lexeme);
}

void SymbolNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode SymbolNode::clone() {
  return std::make_unique<SymbolNode>(*this);
}

pTypeNode SymbolNode::infer_type(TransUnitContext& program) {
  auto stk_id = program.internal.stack->find_symbol(identifier.lexeme);
  if (!stk_id.has_value()) {
    return nullptr;
  }

  auto stk_obj = program.internal.stack->at(stk_id.value());
  if (!stk_obj.has_value()) {
    return nullptr;
  }

  return stk_obj.value().type->clone();
}

// ===============================
// UnaryNode
std::string UnaryNode::to_string(uint32_t& depth) {
  return std::format("Unary<{}>", expression->to_string(depth));
}

void UnaryNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode UnaryNode::clone() {
  return std::make_unique<UnaryNode>(expression->clone());
}

pTypeNode UnaryNode::infer_type(TransUnitContext& program) {
  pTypeNode inner = expression->infer_type(program);
  if (!inner.get()) {
    return nullptr;
  }

  return is_arithmetic(inner) ? std::move(inner) : nullptr;
}

// ===============================
// GroupNode
std::string GroupNode::to_string(uint32_t& depth) {
  return std::format("Group<{}>", expression->to_string(depth));
}

void GroupNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode GroupNode::clone() {
  return std::make_unique<GroupNode>(expression->clone());
}

int GroupNode::precedence() const {
  return std::numeric_limits<int>::max();
}

pTypeNode GroupNode::infer_type(TransUnitContext& program) {
  return expression->infer_type(program);
}

// ===============================
// CallNode
std::string CallNode::to_string(uint32_t& depth) {
  return std::format(
      "CallNode<callee {}, args {}>",
      callee->to_string(depth),
      utils::format_vector<pExprNode>(
          arguments, [&depth](const pExprNode& expr) { return expr->to_string(depth); }
      )
  );
}

void CallNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode CallNode::clone() {
  argument_vector arguments_clone;
  for (pExprNode& argument : arguments) {
    arguments_clone.emplace_back(argument->clone());
  }

  return std::make_unique<CallNode>(callee->clone(), std::move(arguments_clone));
}

pTypeNode CallNode::infer_type(TransUnitContext& program) {
  if (SymbolNode* symbol = get_derived_instance<ExprNode, SymbolNode>(*callee)) {
    auto stk_id = program.internal.stack->find_symbol(symbol->identifier.lexeme);
    if (!stk_id.has_value()) {
      return nullptr;
    }

    auto stk_obj = program.internal.stack->at(stk_id.value());
    if (!stk_obj.has_value()) {
      return nullptr;
    }

    if (FunctionTypeNode* function_type =
            get_derived_instance<TypeNode, FunctionTypeNode>(*stk_obj.value().type)) {
      return function_type->returns->clone();
    }
  }

  return nullptr;
}

// ===============================
// IndexNode
std::string IndexNode::to_string(uint32_t& depth) {
  return std::format(
      "IndexNode<object {}, index {}>", object->to_string(depth), index->to_string(depth)
  );
}

void IndexNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode IndexNode::clone() {
  return std::make_unique<IndexNode>(object->clone(), index->clone());
}

pTypeNode IndexNode::infer_type(TransUnitContext& program) {
  if (SymbolNode* symbol = get_derived_instance<ExprNode, SymbolNode>(*object)) {
    auto stk_id = program.internal.stack->find_symbol(symbol->identifier.lexeme);
    if (!stk_id.has_value()) {
      return nullptr;
    }

    auto stk_obj = program.internal.stack->at(stk_id.value());
    if (!stk_obj.has_value()) {
      return nullptr;
    }

    return stk_obj->type->clone();
  }

  return nullptr;
}

// ===============================
// BinaryNode
std::string BinaryNode::to_string(uint32_t& depth) {
  return std::format(
      "Binary<{} {} {}>",
      lhs_expression->to_string(depth),
      op.lexeme,
      rhs_expression->to_string(depth)
  );
}

void BinaryNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode BinaryNode::clone() {
  return std::make_unique<BinaryNode>(op, lhs_expression->clone(), rhs_expression->clone());
}

pTypeNode BinaryNode::infer_type(TransUnitContext& program) {
  pTypeNode lhs = lhs_expression->infer_type(program);
  pTypeNode rhs = rhs_expression->infer_type(program);

  if (!lhs || !rhs || !is_integral(lhs) || !is_integral(rhs)) {
    return nullptr;
  }

  if (PrimitiveNode* lhs_primitive = get_derived_instance<TypeNode, PrimitiveNode>(*lhs)) {
    if (PrimitiveNode* rhs_primitive = get_derived_instance<TypeNode, PrimitiveNode>(*rhs)) {
      // If either operand is a floating point, result is floating point
      if (lhs_primitive->type == floating_point || rhs_primitive->type == floating_point) {
        return std::make_unique<PrimitiveNode>(lhs_primitive->identifier, floating_point);
      }
      // If both operands are integers, result is integer
      else if (lhs_primitive->type == integer && rhs_primitive->type == integer) {
        return std::make_unique<PrimitiveNode>(lhs_primitive->identifier, integer);
      }
    }
  }

  return nullptr;
}

// ===============================
// TypeCastNode
std::string TypeCastNode::to_string(uint32_t& depth) {
  return std::format(
      "TypeCastNode<{} as {}>", expression->to_string(depth), type->to_string(depth)
  );
}

void TypeCastNode::accept(NodeVisitor& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

pExprNode TypeCastNode::clone() {
  return std::make_unique<TypeCastNode>(expression->clone(), type->clone());
}

pTypeNode TypeCastNode::infer_type(TransUnitContext& program) {
  pTypeNode expr_type = expression->infer_type(program);
  if (!is_castable(expr_type, type)) {
    return nullptr;
  }

  return type->clone();
}

// ===============================
// AutoNode
std::string AutoNode::to_string(uint32_t&) {
  return "AutoNode<>";
}

std::string AutoNode::to_string_x() {
  return "<auto>";
}

void AutoNode::decay(NodeVisitor& visitor, pTypeNode& self) {
  self = visitor.visit(*this);
}

pTypeNode AutoNode::clone() {
  return std::make_unique<AutoNode>(*this);
}

// ===============================
// PrimitiveNode
std::string PrimitiveNode::to_string(uint32_t&) {
  return std::format("PrimitiveNode<{}>", magic_enum::enum_name(type));
}

std::string PrimitiveNode::to_string_x() {
  return std::string(magic_enum::enum_name(type));
}

pTypeNode PrimitiveNode::clone() {
  return std::make_unique<PrimitiveNode>(*this);
}

// ===============================
// GenericNode
std::string GenericNode::to_string(uint32_t& depth) {
  return std::format(
      "GenericNode<{}, {}>",
      identifier.lexeme,
      utils::format_vector<pTypeNode>(
          generics, [&depth](const pTypeNode& elem) { return elem->to_string(depth); }
      )
  );
}

std::string GenericNode::to_string_x() {
  return "<generic-truncated>";
}

void GenericNode::decay(NodeVisitor& visitor, pTypeNode& self) {
  self = visitor.visit(*this);
}

pTypeNode GenericNode::clone() {
  Generics generics_clone;
  for (pTypeNode& generic : generics) {
    generics_clone.emplace_back(generic->clone());
  }

  return std::make_unique<GenericNode>(identifier, std::move(generics_clone), modifiers);
}

// ===============================
// UnionNode
std::string UnionNode::to_string(uint32_t& depth) {
  return std::format("UnionNode<{} & {}>", lhs->to_string(depth), rhs->to_string(depth));
}

std::string UnionNode::to_string_x() {
  return "<union-truncated>";
}

void UnionNode::decay(NodeVisitor& visitor, pTypeNode& self) {
  self = visitor.visit(*this);
}

pTypeNode UnionNode::clone() {
  return std::make_unique<UnionNode>(lhs->clone(), rhs->clone());
}

// ===============================
// FunctionNode
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
  Parameters parameters_clone;
  for (pTypeNode& parameter : parameters) {
    parameters_clone.emplace_back(parameter->clone());
  }

  return std::make_unique<FunctionTypeNode>(std::move(parameters), returns->clone());
}

// ===============================
// DeclarationNode
std::string DeclarationNode::to_string(uint32_t& depth) {
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

void DeclarationNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode DeclarationNode::clone() {
  return std::make_unique<DeclarationNode>(
      is_global, modifiers, identifier, value_expression->clone(), type->clone()
  );
}

// ===============================
// ScopeNode
std::string ScopeNode::to_string(uint32_t& depth) {
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

void ScopeNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode ScopeNode::clone() {
  Statements statements_clone;
  for (pStmtNode& statement : statements) {
    statements_clone.emplace_back(statement->clone());
  }

  return std::make_unique<ScopeNode>(std::move(statements_clone));
}

// ===============================
// FunctionNode
std::string FunctionNode::to_string(uint32_t& depth) {
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

  for (const pStmtNode& stmt : dynamic_cast<ScopeNode&>(*body).statements) {
    oss << stmt->to_string(depth) << "\n";
  }

  depth--;

  oss << DEPTH_TAB_SPACE << "EndFunction<>";
  return oss.str();
}

void FunctionNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode FunctionNode::clone() {
  Parameters parameters_clone;
  for (ParameterNode& parameter : parameters) {
    parameters_clone.emplace_back(
        parameter.identifier, parameter.modifiers, parameter.type->clone()
    );
  }

  return std::make_unique<FunctionNode>(
      is_global, modifiers, identifier, body->clone(), returns->clone(), std::move(parameters_clone)
  );
}

// ===============================
// AssignNode
std::string AssignNode::to_string(uint32_t& depth) {
  return std::format(
      "{}Assign<{} {}= {}>",
      DEPTH_TAB_SPACE,
      augmentation_operator.lexeme,
      assignee->to_string(depth),
      value->to_string(depth)
  );
}

void AssignNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode AssignNode::clone() {
  return std::make_unique<AssignNode>(assignee->clone(), augmentation_operator, value->clone());
}

// ===============================
// FunctionNode
std::string IfNode::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << DEPTH_TAB_SPACE << std::format("IfNode<{}>", condition->to_string(depth)) << "\n";

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

  if (else_node.has_value()) {
    oss << DEPTH_TAB_SPACE << "Else<>"
        << "\n";
    depth++;
    oss << else_node.value()->to_string(depth) << "\n";
    depth--;
    oss << DEPTH_TAB_SPACE << "EndElse<>"
        << "\n";
  }

  depth--;

  oss << DEPTH_TAB_SPACE << "EndIf<>";
  return oss.str();
}

void IfNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode IfNode::clone() {
  ElseIfNodes elseif_nodes_clone;
  for (ElseIfNode& else_if : elseif_nodes) {
    elseif_nodes_clone.emplace_back(else_if.condition->clone(), else_if.scope->clone());
  }

  ElseNode else_node_clone = std::nullopt;
  if (else_node.has_value()) {
    else_node_clone.emplace(else_node.value()->clone());
  }

  return std::make_unique<IfNode>(
      condition->clone(), scope->clone(), std::move(else_node_clone), std::move(elseif_nodes_clone)
  );
}

// ===============================
// ReturnNode

std::string ReturnNode::to_string(uint32_t& depth) {
  return DEPTH_TAB_SPACE + std::format("ReturnNode<{}>", expression->to_string(depth));
}

pStmtNode ReturnNode::clone() {
  return std::make_unique<ReturnNode>(expression->clone());
}

void ReturnNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

// ===============================
// BreakNode

std::string BreakNode::to_string(uint32_t& depth) {
  return DEPTH_TAB_SPACE + "BreakNode<>";
}

pStmtNode BreakNode::clone() {
  return std::make_unique<BreakNode>(token);
}

void BreakNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

// ===============================
// ContinueNode

std::string ContinueNode::to_string(uint32_t& depth) {
  return DEPTH_TAB_SPACE + "ContinueNode<>";
}

pStmtNode ContinueNode::clone() {
  return std::make_unique<ContinueNode>(token);
}

void ContinueNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

// ===============================
// WhileNode
std::string WhileNode::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << DEPTH_TAB_SPACE << std::format("WhileNode<{}>", condition->to_string(depth)) << "\n";

  depth++;

  for (const pStmtNode& stmt : dynamic_cast<ScopeNode&>(*body).statements) {
    oss << stmt->to_string(depth) << "\n";
  }

  depth--;

  oss << DEPTH_TAB_SPACE << "EndWhile<>";
  return oss.str();
}

void WhileNode::accept(NodeVisitor& visitor) {
  visitor.visit(*this);
}

pStmtNode WhileNode::clone() {
  return std::make_unique<WhileNode>(condition->clone(), body->clone());
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
