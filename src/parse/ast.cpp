// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ast.h"

#include <codegen/visitor.h>
#include <codegen/types.h>
#include <interpreter/tvalue.h>

#define depth_tab_space std::string(depth, ' ')

namespace via {

using enum Value::Tag;

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
    return "Literal<Nil>";
  }
}

void LitExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* LitExprNode::infer_type(TransUnitContext& unit_ctx) {
  Value::Tag val_ty = std::visit(
    [](auto&& value) {
      using T = std::decay_t<decltype(value)>;
      return DataType<T>::type;
    },
    value
  );

  return unit_ctx.internal.ast_allocator.emplace<PrimTypeNode>(value_token, val_ty);
}

// ===============================
// SymExprNode
std::string SymExprNode::to_string(uint32_t&) {
  return std::format("Symbol<{}>", identifier.lexeme);
}

void SymExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* SymExprNode::infer_type(TransUnitContext& unit_ctx) {
  auto& current_closure = unit_ctx.internal.function_stack.back();
  auto stk_id = current_closure.locals.get_local_by_symbol(identifier.lexeme);
  if (stk_id.has_value()) {
    return (*stk_id)->type;
  }

  auto global = unit_ctx.internal.globals.get_global(identifier.lexeme);
  if (global.has_value()) {
    return global->type;
  }

  if (unit_ctx.internal.function_stack.size() > 0) {
    auto& top_function = unit_ctx.internal.function_stack.back();
    for (size_t i = 0; i < top_function.decl->parameters.size(); i++) {
      const ParamStmtNode& param = top_function.decl->parameters[i];
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

void UnaryExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* UnaryExprNode::infer_type(TransUnitContext& unit_ctx) {
  TypeNodeBase* inner = expression->infer_type(unit_ctx);
  if (!inner) {
    return nullptr;
  }

  if (op.type == TokenType::OP_LEN)
    return unit_ctx.internal.ast_allocator.emplace<PrimTypeNode>(Token(), Int);
  else if (op.type == TokenType::OP_INC || op.type == TokenType::OP_DEC || op.type == TokenType::OP_SUB)
    return inner;

  return nullptr;
}

// ===============================
// GroupExprNode
std::string GroupExprNode::to_string(uint32_t& depth) {
  return std::format("Group<{}>", expression->to_string(depth));
}

void GroupExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
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
    format_vector<ExprNodeBase*>(
      arguments, [&depth](ExprNodeBase* expr) { return expr->to_string(depth); }
    )
  );
}

void CallExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* CallExprNode::infer_type(TransUnitContext& unit_ctx) {
  if (SymExprNode* symbol = dynamic_cast<SymExprNode*>(callee)) {
    TypeNodeBase* ty = symbol->infer_type(unit_ctx);
    if (FunctionTypeNode* fn_ty = dynamic_cast<FunctionTypeNode*>(ty)) {
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

void IndexExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
  visitor.visit(*this, dst);
}

TypeNodeBase* IndexExprNode::infer_type(TransUnitContext& unit_ctx) {
  auto& current_closure = unit_ctx.internal.function_stack.back();
  if (SymExprNode* symbol = dynamic_cast<SymExprNode*>(object)) {
    auto stk_id = current_closure.locals.get_local_by_symbol(symbol->identifier.lexeme);
    if (!stk_id.has_value()) {
      return nullptr;
    }

    return (*stk_id)->type;
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

void BinExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
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
  if (PrimTypeNode* lhs_primitive = dynamic_cast<PrimTypeNode*>(lhs)) {
    if (PrimTypeNode* rhs_primitive = dynamic_cast<PrimTypeNode*>(rhs)) {
      // Check for floating-point types
      if (lhs_primitive->type == Float || rhs_primitive->type == Float) {
        return unit_ctx.internal.ast_allocator.emplace<PrimTypeNode>(
          lhs_primitive->identifier, Float
        );
      }
      // Check for Int types
      else if (lhs_primitive->type == Int && rhs_primitive->type == Int) {
        return unit_ctx.internal.ast_allocator.emplace<PrimTypeNode>(
          lhs_primitive->identifier, Int
        );
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

void CastExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
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
  return std::format("StepExprNode<{}, {}>", target->to_string(depth), is_increment ? "++" : "--");
}

TypeNodeBase* StepExprNode::infer_type(TransUnitContext& unit_ctx) {
  return target->infer_type(unit_ctx);
}

void StepExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
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
  if (values.empty()) {
    return nullptr;
  }

  ExprNodeBase* first_expr = values.front();
  TypeNodeBase* first_type = first_expr->infer_type(unit_ctx);

  for (ExprNodeBase* expr : values) {
    if (!is_same(first_type, expr->infer_type(unit_ctx))) {
      return nullptr;
    }
  }

  return unit_ctx.internal.ast_allocator.emplace<ArrayTypeNode>(first_type);
}

void ArrayExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
  visitor.visit(*this, dst);
}

std::string IntrinsicExprNode::to_string(uint32_t& depth) {
  return std::format(
    "IntrinsicExprNode<{}, {}>",
    intrinsic.lexeme,
    format_vector<ExprNodeBase*>(
      exprs, [&depth](ExprNodeBase* expr) { return expr->to_string(depth); }
    )
  );
}

TypeNodeBase* IntrinsicExprNode::infer_type(TransUnitContext& unit_ctx) {
  if (intrinsic.lexeme == "type" || intrinsic.lexeme == "typeof" || intrinsic.lexeme == "nameof") {
    return unit_ctx.internal.ast_allocator.emplace<PrimTypeNode>(
      Token(TokenType::IDENTIFIER, "string", 0, 0, 0), String
    );
  }

  return unit_ctx.internal.ast_allocator.emplace<PrimTypeNode>(Token(), Nil);
}

void IntrinsicExprNode::accept(NodeVisitorBase& visitor, operand_t dst) {
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
    format_vector<TypeNodeBase*>(
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
// ArrayTypeNode
std::string ArrayTypeNode::to_string(uint32_t& depth) {
  return std::format("ArrayTypeNode<{}>", type->to_string(depth));
}

std::string ArrayTypeNode::to_output_string() {
  return apply_color("[", fg_color::cyan) + type->to_output_string()
    + apply_color("]", fg_color::cyan);
}

void ArrayTypeNode::decay(NodeVisitorBase& visitor, TypeNodeBase*& self) {
  self = visitor.visit(*this);
}

// ===============================
// FuncDeclStmtNode
std::string FunctionTypeNode::to_string(uint32_t& depth) {
  return std::format(
    "FunctionTypeNode<{} -> {}>",
    format_vector<ParamStmtNode>(
      parameters, [](const ParamStmtNode& elem) { return elem.type->to_output_string(); }
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

//  ===============
// [ ParamStmtNode ]
//  ===============
std::string ParamStmtNode::to_string(uint32_t& depth) {
  return std::format(
    "{}Parameter<{}: {}>", depth_tab_space, identifier.lexeme, type->to_string(depth)
  );
}

void ParamStmtNode::accept(NodeVisitorBase&) {}

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
    rvalue->to_string(depth)
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
           "Function<{} {} {}>\n",
           is_global ? "global" : "local",
           modifs.to_string(),
           identifier.lexeme
         );

  depth++;

  for (const ParamStmtNode& parameter : parameters) {
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
    lvalue->to_string(depth),
    rvalue->to_string(depth)
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

  for (const ElseIfNode* else_if : elseif_nodes) {
    oss << depth_tab_space << std::format("ElseIf<{}>", else_if->condition->to_string(depth))
        << "\n";

    depth++;

    oss << else_if->scope->to_string(depth) << "\n";

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
// DeferStmtNode
std::string DeferStmtNode::to_string(uint32_t& depth) {
  uint32_t zero_depth = 0;
  return std::format("{}DeferStmtNode<{}>", depth_tab_space, stmt->to_string(zero_depth));
}

void DeferStmtNode::accept(NodeVisitorBase& visitor) {
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
