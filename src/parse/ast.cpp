//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "ast.h"
#include "compiler-types.h"
#include "visitor.h"
#include "stack.h"
#include "format-vector.h"
#include "color.h"

#define depth_tab_space std::string(depth, ' ')

namespace via {

using enum IValueType;
using namespace utils;

std::string StmtModifiers::to_string() const {
  return std::format("{}", is_const ? "const" : "");
}

// ===============================
// LitExprNode
std::string LitExprNode::to_string(uint32_t&) {
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

void LitExprNode::accept(NodeVisitorBase& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* LitExprNode::infer_type(TransUnitContext& unit_ctx) {
  IValueType IValueType = std::visit(
    [](auto&& value) {
      using T = std::decay_t<decltype(value)>;
      return DataType<T>::type;
    },
    value
  );

  return unit_ctx.ast->allocator.emplace<PrimTypeNode>(value_token, IValueType);
}

// ===============================
// SymExprNode
std::string SymExprNode::to_string(uint32_t&) {
  return std::format("Symbol<{}>", identifier.lexeme);
}

void SymExprNode::accept(NodeVisitorBase& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* SymExprNode::infer_type(TransUnitContext& unit_ctx) {
  auto stk_id = unit_ctx.internal.variable_stack->find_symbol(identifier.lexeme);
  if (stk_id.has_value()) {
    auto stk_obj = unit_ctx.internal.variable_stack->at(stk_id.value());
    if (stk_obj.has_value()) {
      return stk_obj->type;
    }

    return nullptr;
  }

  auto global = unit_ctx.internal.globals->get_global(identifier.lexeme);
  if (global.has_value()) {
    return global->type;
  }

  if (unit_ctx.internal.function_stack->size() > 0) {
    auto& top_function = unit_ctx.internal.function_stack->top();
    for (size_t i = 0; i < top_function.func_stmt->parameters.size(); i++) {
      const ParamNode& param = top_function.func_stmt->parameters[i];
      if (param.identifier.lexeme == identifier.lexeme) {
        return param.type;
      }
    }
  }

  return nullptr;
}

// ===============================
// UnaryExprNode
std::string UnaryExprNode::to_string(uint32_t& depth) {
  return std::format("Unary<{}>", expression->to_string(depth));
}

void UnaryExprNode::accept(NodeVisitorBase& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* UnaryExprNode::infer_type(TransUnitContext& unit_ctx) {
  TypeNodeBase* inner = expression->infer_type(unit_ctx);
  if (!inner) {
    return nullptr;
  }

  return is_arithmetic(inner) ? inner : nullptr;
}

// ===============================
// GroupExprNode
std::string GroupExprNode::to_string(uint32_t& depth) {
  return std::format("Group<{}>", expression->to_string(depth));
}

void GroupExprNode::accept(NodeVisitorBase& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

int GroupExprNode::precedence() const {
  return std::numeric_limits<int>::max();
}

TypeNodeBase* GroupExprNode::infer_type(TransUnitContext& unit_ctx) {
  return expression->infer_type(unit_ctx);
}

// ===============================
// CallExprNode
std::string CallExprNode::to_string(uint32_t& depth) {
  return std::format(
    "CallExprNode<callee {}, args {}>",
    callee->to_string(depth),
    utils::format_vector<ExprNodeBase*>(
      arguments, [&depth](ExprNodeBase* expr) { return expr->to_string(depth); }
    )
  );
}

void CallExprNode::accept(NodeVisitorBase& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* CallExprNode::infer_type(TransUnitContext& unit_ctx) {
  if (SymExprNode* symbol = get_derived_instance<ExprNodeBase, SymExprNode>(*callee)) {
    TypeNodeBase* ty = symbol->infer_type(unit_ctx);
    if (FunctionTypeNode* fn_ty = get_derived_instance<TypeNodeBase, FunctionTypeNode>(*ty)) {
      return fn_ty->returns;
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

void IndexExprNode::accept(NodeVisitorBase& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* IndexExprNode::infer_type(TransUnitContext& unit_ctx) {
  if (SymExprNode* symbol = get_derived_instance<ExprNodeBase, SymExprNode>(*object)) {
    auto stk_id = unit_ctx.internal.variable_stack->find_symbol(symbol->identifier.lexeme);
    if (!stk_id.has_value()) {
      return nullptr;
    }

    auto stk_obj = unit_ctx.internal.variable_stack->at(stk_id.value());
    if (!stk_obj.has_value()) {
      return nullptr;
    }

    return stk_obj->type;
  }

  return nullptr;
}

// ===============================
// BinExprNode
std::string BinExprNode::to_string(uint32_t& depth) {
  return std::format(
    "Binary<{} {} {}>",
    lhs_expression->to_string(depth),
    op.lexeme,
    rhs_expression->to_string(depth)
  );
}

void BinExprNode::accept(NodeVisitorBase& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* BinExprNode::infer_type(TransUnitContext& unit_ctx) {
  TypeNodeBase* lhs = lhs_expression->infer_type(unit_ctx);
  TypeNodeBase* rhs = rhs_expression->infer_type(unit_ctx);

  // Early return if either lhs or rhs is invalid or not arithmetic
  if (!lhs || !rhs || !is_arithmetic(lhs) || !is_arithmetic(rhs)) {
    return nullptr;
  }

  // Check for valid primitive types in both lhs and rhs
  if (PrimTypeNode* lhs_primitive = get_derived_instance<TypeNodeBase, PrimTypeNode>(*lhs)) {
    if (PrimTypeNode* rhs_primitive = get_derived_instance<TypeNodeBase, PrimTypeNode>(*rhs)) {
      // Check for floating-point types
      if (lhs_primitive->type == floating_point || rhs_primitive->type == floating_point) {
        return unit_ctx.ast->allocator.emplace<PrimTypeNode>(
          lhs_primitive->identifier, floating_point
        );
      }
      // Check for integer types
      else if (lhs_primitive->type == integer && rhs_primitive->type == integer) {
        return unit_ctx.ast->allocator.emplace<PrimTypeNode>(lhs_primitive->identifier, integer);
      }
    }
  }

  // Handle cases where the type inference fails
  return nullptr;
}

// ===============================
// CastExprNode
std::string CastExprNode::to_string(uint32_t& depth) {
  return std::format(
    "CastExprNode<{} as {}>", expression->to_string(depth), type->to_string(depth)
  );
}

void CastExprNode::accept(NodeVisitorBase& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* CastExprNode::infer_type(TransUnitContext& unit_ctx) {
  TypeNodeBase* expr_type = expression->infer_type(unit_ctx);
  if (!is_castable(expr_type, type)) {
    return nullptr;
  }

  return type;
}

// StepExprNode
std::string StepExprNode::to_string(uint32_t& depth) {
  return std::format(
    "StepExprNode<{}, {}, ispostfix: {}>",
    target->to_string(depth),
    is_increment ? "++" : "--",
    is_postfix
  );
}

TypeNodeBase* StepExprNode::infer_type(TransUnitContext& unit_ctx) {
  return target->infer_type(unit_ctx);
}

void StepExprNode::accept(NodeVisitorBase& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

// ArrayExprNode
std::string ArrayExprNode::to_string(uint32_t& depth) {
  return std::format(
    "ArrayExprNode<{}>",
    format_vector<ExprNodeBase*>(
      values, [&depth](ExprNodeBase* pexpr) { return pexpr->to_string(depth); }
    )
  );
}

TypeNodeBase* ArrayExprNode::infer_type(TransUnitContext& unit_ctx) {
  return unit_ctx.ast->allocator.emplace<PrimTypeNode>(Token(), nil);
}

void ArrayExprNode::accept(NodeVisitorBase& visitor, uint32_t dst) {
  visitor.visit(*this, dst);
}

// ===============================
// AutoTypeNode
std::string AutoTypeNode::to_string(uint32_t&) {
  return "AutoTypeNode<>";
}

std::string AutoTypeNode::to_output_string() {
  return apply_color("<auto>", fg_color::magenta);
}

void AutoTypeNode::decay(NodeVisitorBase& visitor, TypeNodeBase*& self) {
  self = visitor.visit(*this);
}

// ===============================
// PrimTypeNode
std::string PrimTypeNode::to_string(uint32_t&) {
  return std::format("PrimTypeNode<{}>", magic_enum::enum_name(type));
}

std::string PrimTypeNode::to_output_string() {
  std::string enum_name = std::string(magic_enum::enum_name(type));
  return apply_color(enum_name, fg_color::magenta);
}

// ===============================
// GenericTypeNode
std::string GenericTypeNode::to_string(uint32_t& depth) {
  return std::format(
    "GenericTypeNode<{}, {}>",
    identifier.lexeme,
    utils::format_vector<TypeNodeBase*>(
      generics, [&depth](TypeNodeBase* elem) { return elem->to_string(depth); }
    )
  );
}

std::string GenericTypeNode::to_output_string() {
  return apply_color("<generic-truncated>", fg_color::magenta);
}

void GenericTypeNode::decay(NodeVisitorBase& visitor, TypeNodeBase*& self) {
  self = visitor.visit(*this);
}

// ===============================
// UnionTypeNode
std::string UnionTypeNode::to_string(uint32_t& depth) {
  return std::format("UnionTypeNode<{} & {}>", lhs->to_string(depth), rhs->to_string(depth));
}

std::string UnionTypeNode::to_output_string() {
  return apply_color("<union-truncated>", fg_color::magenta);
}

void UnionTypeNode::decay(NodeVisitorBase& visitor, TypeNodeBase*& self) {
  self = visitor.visit(*this);
}

// ===============================
// FuncDeclStmtNode
std::string FunctionTypeNode::to_string(uint32_t& depth) {
  return std::format(
    "FunctionTypeNode<{} -> {}>",
    utils::format_vector<ParamNode>(
      parameters, [](const ParamNode& elem) { return elem.type->to_output_string(); }
    ),
    returns->to_string(depth)
  );
}

std::string FunctionTypeNode::to_output_string() {
  return apply_color("<function-truncated>", fg_color::magenta);
}

void FunctionTypeNode::decay(NodeVisitorBase& visitor, TypeNodeBase*& self) {
  self = visitor.visit(*this);
}

// ===============================
// DeclStmtNode
std::string DeclStmtNode::to_string(uint32_t& depth) {
  return std::format(
    "{}Declaration<{} {} {}: {} = {}>",
    depth_tab_space,
    is_global ? "global" : "local",
    modifs.to_string(),
    identifier.lexeme,
    type->to_string(depth),
    value_expression->to_string(depth)
  );
}

void DeclStmtNode::accept(NodeVisitorBase& visitor) {
  visitor.visit(*this);
}

// ===============================
// ScopeStmtNode
std::string ScopeStmtNode::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << depth_tab_space << "Scope<>\n";

  depth++;

  for (StmtNodeBase*& pstmt : statements) {
    oss << pstmt->to_string(depth) << "\n";
  }

  depth--;

  oss << depth_tab_space << "EndScope<>";
  return oss.str();
}

void ScopeStmtNode::accept(NodeVisitorBase& visitor) {
  visitor.visit(*this);
}

// ===============================
// FuncDeclStmtNode
std::string FuncDeclStmtNode::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << depth_tab_space
      << std::format(
           "IFunction<{} {} {}>\n",
           is_global ? "global" : "local",
           modifs.to_string(),
           identifier.lexeme
         );

  depth++;

  for (const ParamNode& parameter : parameters) {
    oss << depth_tab_space << std::format("Parameter<{}>", parameter.identifier.lexeme) << "\n";
  }

  for (StmtNodeBase*& stmt : dynamic_cast<ScopeStmtNode&>(*body).statements) {
    oss << stmt->to_string(depth) << "\n";
  }

  depth--;

  oss << depth_tab_space << "EndFunction<>";
  return oss.str();
}

void FuncDeclStmtNode::accept(NodeVisitorBase& visitor) {
  visitor.visit(*this);
}

// ===============================
// AssignStmtNode
std::string AssignStmtNode::to_string(uint32_t& depth) {
  return std::format(
    "{}Assign<{} {}= {}>",
    depth_tab_space,
    augmentation_operator.lexeme,
    assignee->to_string(depth),
    value->to_string(depth)
  );
}

void AssignStmtNode::accept(NodeVisitorBase& visitor) {
  visitor.visit(*this);
}

// ===============================
// FuncDeclStmtNode
std::string IfStmtNode::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << depth_tab_space << std::format("IfStmtNode<{}>", condition->to_string(depth)) << "\n";

  depth++;

  oss << scope->to_string(depth) << "\n";

  for (const elseif_node& elseif_node : elseif_nodes) {
    oss << depth_tab_space << std::format("ElseIf<{}>", elseif_node.condition->to_string(depth))
        << "\n";

    depth++;

    oss << elseif_node.scope->to_string(depth) << "\n";

    depth--;
    oss << depth_tab_space << "EndElseIf<>\n";
  }

  if (else_node) {
    oss << depth_tab_space << "Else<>"
        << "\n";
    depth++;
    oss << else_node->to_string(depth) << "\n";
    depth--;
    oss << depth_tab_space << "EndElse<>"
        << "\n";
  }

  depth--;

  oss << depth_tab_space << "EndIf<>";
  return oss.str();
}

void IfStmtNode::accept(NodeVisitorBase& visitor) {
  visitor.visit(*this);
}

// ===============================
// ReturnStmtNode

std::string ReturnStmtNode::to_string(uint32_t& depth) {
  return depth_tab_space + std::format("ReturnStmtNode<{}>", expression->to_string(depth));
}

void ReturnStmtNode::accept(NodeVisitorBase& visitor) {
  visitor.visit(*this);
}

// ===============================
// BreakStmtNode

std::string BreakStmtNode::to_string(uint32_t& depth) {
  return depth_tab_space + "BreakStmtNode<>";
}

void BreakStmtNode::accept(NodeVisitorBase& visitor) {
  visitor.visit(*this);
}

// ===============================
// ContinueStmtNode

std::string ContinueStmtNode::to_string(uint32_t& depth) {
  return depth_tab_space + "ContinueStmtNode<>";
}

void ContinueStmtNode::accept(NodeVisitorBase& visitor) {
  visitor.visit(*this);
}

// ===============================
// WhileStmtNode
std::string WhileStmtNode::to_string(uint32_t& depth) {
  std::ostringstream oss;
  oss << depth_tab_space << std::format("WhileStmtNode<{}>", condition->to_string(depth)) << "\n";

  depth++;

  for (StmtNodeBase*& stmt : dynamic_cast<ScopeStmtNode&>(*body).statements) {
    oss << stmt->to_string(depth) << "\n";
  }

  depth--;

  oss << depth_tab_space << "EndWhile<>";
  return oss.str();
}

void WhileStmtNode::accept(NodeVisitorBase& visitor) {
  visitor.visit(*this);
}

// ===============================
// ExprStmtNode
std::string ExprStmtNode::to_string(uint32_t& depth) {
  return std::format("{}ExprStmtNode<{}>", depth_tab_space, expression->to_string(depth));
}

void ExprStmtNode::accept(NodeVisitorBase& visitor) {
  visitor.visit(*this);
}

} // namespace via
