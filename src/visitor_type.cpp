// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "visitor.h"
#include "bytecode-builder.h"

namespace via {

using namespace sema;

void TypeNodeVisitor::visit(NodeDeclStmt& declaration_node) {
  TypeNode* infered_type = resolve_type(ctx, declaration_node.rvalue);
  TypeNode* annotated_type = declaration_node.type;

  VIA_CHECK_INFERED(infered_type, declaration_node.rvalue);
  VIA_CHECK_INFERED(annotated_type, declaration_node.rvalue);

  if (is_nil(annotated_type)) {
    // Warning: "Nil-typed-variable"
    auto message = std::format("Variable typed as {}", apply_color("Nil", fg_color::magenta));
    warning(ctx, declaration_node.type->begin, declaration_node.type->end, message);
    info(ctx, "'Nil' typed variables are incapable of holding more than one value");
    flush(ctx);
  }

  if (!is_compatible(infered_type, annotated_type)) {
    // Error: "variable-decl-type-mismatch"
    auto message = std::format(
      "Variable initialized with type {} which does not match with declaration type {}",
      infered_type->to_output_string(),
      annotated_type->to_output_string()
    );
    error(ctx, declaration_node.rvalue->begin, declaration_node.rvalue->end, message);
    flush(ctx);
  }
}

void TypeNodeVisitor::visit(NodeAsgnStmt& assign_node) {
  TypeNode* infered_type = resolve_type(ctx, assign_node.lvalue);
  TypeNode* assigned_type = resolve_type(ctx, assign_node.rvalue);

  VIA_CHECK_INFERED(infered_type, assign_node.lvalue);
  VIA_CHECK_INFERED(assigned_type, assign_node.rvalue);

  if (!is_compatible(infered_type, assigned_type)) {
    // Error: "lvalue-bind-type-mismatch"
    auto message = std::format(
      "Assigning incompatible rvalue of type {} to lvalue declared as {}",
      assigned_type->to_output_string(),
      infered_type->to_output_string()
    );
    error(ctx, assign_node.rvalue->begin, assign_node.rvalue->end, message);
    flush(ctx);
  }
}

void TypeNodeVisitor::visit(NodeFuncDeclStmt&) {}

} // namespace via
