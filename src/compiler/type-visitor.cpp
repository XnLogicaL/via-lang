//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "visitor.h"
#include "compiler-types.h"
#include "color.h"
#include "ast.h"

namespace via {

using namespace utils;

void type_node_visitor::visit(DeclStmtNode& declaration_node) {
  TypeNodeBase* infered_type = declaration_node.rvalue->infer_type(unit_ctx);
  TypeNodeBase*& annotated_type = declaration_node.type;

  VIA_TINFERENCE_FAILURE(infered_type, declaration_node.rvalue);
  VIA_TINFERENCE_FAILURE(annotated_type, declaration_node.rvalue);

  if (is_nil(annotated_type)) {
    compiler_warning(
      declaration_node.type->begin,
      declaration_node.type->end,
      std::format(
        "Variable implicitly or explicitly typed as {}", apply_color("nil", fg_color::magenta)
      )
    );

    compiler_info("A 'nil' variable cannot be used meaningfully and serves no functional purpose. "
                  "This may indicate a logical problem in the code.");
    compiler_output_end();
  }

  if (!is_compatible(infered_type, annotated_type)) {
    compiler_error(
      declaration_node.rvalue->begin,
      declaration_node.rvalue->end,
      std::format(
        "Variable initializer type {} does not match with annotated type {}",
        infered_type->to_output_string(),
        annotated_type->to_output_string()
      )
    );
    compiler_output_end();
  }
}

void type_node_visitor::visit(AssignStmtNode& assign_node) {
  TypeNodeBase* infered_type = assign_node.lvalue->infer_type(unit_ctx);
  TypeNodeBase* assigned_type = assign_node.rvalue->infer_type(unit_ctx);

  VIA_TINFERENCE_FAILURE(infered_type, assign_node.lvalue);
  VIA_TINFERENCE_FAILURE(assigned_type, assign_node.rvalue);

  if (!is_compatible(infered_type, assigned_type)) {
    compiler_error(
      assign_node.rvalue->begin,
      assign_node.rvalue->end,
      std::format(
        "Assigning incompatible rvalue of type {} to variable declared with type {}",
        assigned_type->to_output_string(),
        infered_type->to_output_string()
      )
    );
    compiler_output_end();
  }
}

void type_node_visitor::visit(FuncDeclStmtNode&) {}

} // namespace via
