// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"
#include "visitor.h"
#include "types.h"
#include "ast.h"

#define CHECK_TYPE_INFERENCE_FAILURE(type, expr)                                                   \
    if (!type.get()) {                                                                             \
        visitor_failed = true;                                                                     \
        emitter.out_range(expr->begin, expr->end, "Expression type could not be infered", Error);  \
        emitter.out_flat(                                                                          \
            "This error message likely indicates an internal compiler bug. Please create an "      \
            "issue "                                                                               \
            "at https://github.com/XnLogicaL/via-lang",                                            \
            Info                                                                                   \
        );                                                                                         \
        return;                                                                                    \
    }

VIA_NAMESPACE_BEGIN

using enum OutputSeverity;

void TypeVisitor::visit(DeclarationNode& declaration_node) {
    pTypeNode  infered_type   = declaration_node.value_expression->infer_type(program);
    pTypeNode& annotated_type = declaration_node.type;

    CHECK_TYPE_INFERENCE_FAILURE(infered_type, declaration_node.value_expression);
    CHECK_TYPE_INFERENCE_FAILURE(annotated_type, declaration_node.value_expression);

    if (!is_compatible(*infered_type, *annotated_type)) {
        visitor_failed = true;
        emitter.out_range(
            declaration_node.value_expression->begin,
            declaration_node.value_expression->end,
            std::format(
                "Expression type '{}' is not related to or implicitly castable into annotated type "
                "'{}'",
                infered_type->to_string_x(),
                annotated_type->to_string_x()
            ),
            Error
        );
    }
}

void TypeVisitor::visit(AssignNode& assign_node) {
    pTypeNode infered_type  = assign_node.assignee->infer_type(program);
    pTypeNode assigned_type = assign_node.value->infer_type(program);

    CHECK_TYPE_INFERENCE_FAILURE(infered_type, assign_node.assignee);
    CHECK_TYPE_INFERENCE_FAILURE(assigned_type, assign_node.value);

    if (!is_compatible(*infered_type, *assigned_type)) {
        visitor_failed = true;
        emitter.out_range(
            assign_node.value->begin,
            assign_node.value->end,
            std::format(
                "Assigning incompatible type '{}' to an lvalue that holds type '{}'",
                assigned_type->to_string_x(),
                infered_type->to_string_x()
            ),
            Error
        );
    }
}

void TypeVisitor::visit(FunctionNode&) {}

VIA_NAMESPACE_END
