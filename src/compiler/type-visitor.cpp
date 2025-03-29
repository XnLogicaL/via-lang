// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "visitor.h"
#include "compiler-types.h"
#include "ast.h"

namespace via {

void type_node_visitor::visit(decl_stmt_node& declaration_node) {
  p_type_node_t infered_type = declaration_node.value_expression->infer_type(unit_ctx);
  p_type_node_t& annotated_type = declaration_node.type;

  vl_tinference_failure(infered_type, declaration_node.value_expression);
  vl_tinference_failure(annotated_type, declaration_node.value_expression);

  if (!is_compatible(infered_type, annotated_type)) {
    compiler_error(
      declaration_node.value_expression->begin,
      declaration_node.value_expression->end,
      std::format(
        "Variable initializer type '{}' does not match with annotated type '{}'",
        infered_type->to_string_x(),
        annotated_type->to_string_x()
      )
    );
  }
}

void type_node_visitor::visit(assign_stmt_node& assign_node) {
  p_type_node_t infered_type = assign_node.assignee->infer_type(unit_ctx);
  p_type_node_t assigned_type = assign_node.value->infer_type(unit_ctx);

  vl_tinference_failure(infered_type, assign_node.assignee);
  vl_tinference_failure(assigned_type, assign_node.value);

  if (!is_compatible(infered_type, assigned_type)) {
    compiler_error(
      assign_node.value->begin,
      assign_node.value->end,
      std::format(
        "Assigning incompatible type '{}' to variable declared with type '{}'",
        assigned_type->to_string_x(),
        infered_type->to_string_x()
      )
    );
  }
}

void type_node_visitor::visit(func_stmt_node&) {}

} // namespace via
