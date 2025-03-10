// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"
#include "visitor.h"
#include "types.h"
#include "ast.h"

VIA_NAMESPACE_BEGIN

using enum OutputSeverity;

void TypeVisitor::visit(DeclarationNode& declaration_node) {
    pTypeNode  infered_type   = declaration_node.value_expression->infer_type(program);
    pTypeNode& annotated_type = declaration_node.type;

    if (!infered_type.get() || !annotated_type.get()) {
        visitor_failed = true;
        emitter.out_range(
            declaration_node.value_expression->begin,
            declaration_node.value_expression->end,
            "Expression type could not be infered",
            Error
        );
        return;
    }

    if (!is_compatible(*infered_type, *annotated_type)) {
        visitor_failed = true;
        emitter.out_range(
            declaration_node.value_expression->begin,
            declaration_node.value_expression->end,
            std::format(
                "Expression type '{}' is not related to or castable into annotated type '{}'",
                infered_type->to_string_x(),
                annotated_type->to_string_x()
            ),
            Error
        );
    }
}

void TypeVisitor::visit(FunctionNode&) {}

VIA_NAMESPACE_END
