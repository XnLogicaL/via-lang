// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"
#include "visitor.h"
#include "types.h"
#include "ast.h"

VIA_NAMESPACE_BEGIN

using enum OutputSeverity;

pTypeNode DecayVisitor::visit(AutoNode& auto_node, const pExprNode& expression) {
    if (LiteralNode* literal = get_derived_instance<ExprNode, LiteralNode>(*expression)) {
        pTypeNode new_type = literal->infer_type(program);
        if (!new_type.get()) {
            visitor_failed = true;
            emitter.out(
                literal->value_token.position, "Failed to infer type of literal expression", Error
            );
            return std::make_unique<AutoNode>(auto_node);
        }
        return new_type;
    }

    VIA_UNREACHABLE;
}

VIA_NAMESPACE_END
