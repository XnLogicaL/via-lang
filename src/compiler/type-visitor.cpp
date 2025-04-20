// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "visitor.h"
#include "compiler.h"

namespace via {

using namespace compiler_util;

void TypeNodeVisitor::visit(DeclStmtNode& declaration_node) {
  TypeNodeBase* infered_type = resolve_type(ctx, declaration_node.rvalue);
  TypeNodeBase* annotated_type = declaration_node.type;

  CHECK_INFERED_TYPE(infered_type, declaration_node.rvalue);
  CHECK_INFERED_TYPE(annotated_type, declaration_node.rvalue);

  if (is_nil(annotated_type)) {
    // Warning: "Nil-typed-variable"
    auto message = std::format("Variable typed as {}", apply_color("Nil", fg_color::magenta));
    compiler_warning(ctx, declaration_node.type->begin, declaration_node.type->end, message);
    compiler_info(ctx, "'Nil' typed variables are incapable of holding more than one value");
    compiler_output_end(ctx);
  }

  if (!is_compatible(infered_type, annotated_type)) {
    // Error: "variable-decl-type-mismatch"
    auto message = std::format(
      "Variable initialized with type {} which does not match with declaration type {}",
      infered_type->to_output_string(),
      annotated_type->to_output_string()
    );
    compiler_error(ctx, declaration_node.rvalue->begin, declaration_node.rvalue->end, message);
    compiler_output_end(ctx);
  }
}

void TypeNodeVisitor::visit(AssignStmtNode& assign_node) {
  TypeNodeBase* infered_type = resolve_type(ctx, assign_node.lvalue);
  TypeNodeBase* assigned_type = resolve_type(ctx, assign_node.rvalue);

  CHECK_INFERED_TYPE(infered_type, assign_node.lvalue);
  CHECK_INFERED_TYPE(assigned_type, assign_node.rvalue);

  if (!is_compatible(infered_type, assigned_type)) {
    // Error: "lvalue-bind-type-mismatch"
    auto message = std::format(
      "Assigning incompatible rvalue of type {} to lvalue declared as {}",
      assigned_type->to_output_string(),
      infered_type->to_output_string()
    );
    compiler_error(ctx, assign_node.rvalue->begin, assign_node.rvalue->end, message);
    compiler_output_end(ctx);
  }
}

void TypeNodeVisitor::visit(FuncDeclStmtNode&) {}

} // namespace via
