// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "visitor.h"
#include "compiler-types.h"
#include "ast.h"

VIA_NAMESPACE_BEGIN

void TypeVisitor::visit(DeclarationNode& declaration_node) {
  pTypeNode  infered_type   = declaration_node.value_expression->infer_type(unit_ctx);
  pTypeNode& annotated_type = declaration_node.type;

  CHECK_TYPE_INFERENCE_FAILURE(infered_type, declaration_node.value_expression);
  CHECK_TYPE_INFERENCE_FAILURE(annotated_type, declaration_node.value_expression);

  if (!is_compatible(infered_type, annotated_type)) {
    compiler_error(
        declaration_node.value_expression->begin,
        declaration_node.value_expression->end,
        std::format(
            "Expression type '{}' is not related to or implicitly castable into annotated type "
            "'{}'",
            infered_type->to_string_x(),
            annotated_type->to_string_x()
        )
    );
  }
}

void TypeVisitor::visit(AssignNode& assign_node) {
  pTypeNode infered_type  = assign_node.assignee->infer_type(unit_ctx);
  pTypeNode assigned_type = assign_node.value->infer_type(unit_ctx);

  CHECK_TYPE_INFERENCE_FAILURE(infered_type, assign_node.assignee);
  CHECK_TYPE_INFERENCE_FAILURE(assigned_type, assign_node.value);

  if (!is_compatible(infered_type, assigned_type)) {
    compiler_error(
        assign_node.value->begin,
        assign_node.value->end,
        std::format(
            "Assigning incompatible type '{}' to an lvalue that holds type '{}'",
            assigned_type->to_string_x(),
            infered_type->to_string_x()
        )
    );
  }
}

void TypeVisitor::visit(FunctionNode&) {}

VIA_NAMESPACE_END
