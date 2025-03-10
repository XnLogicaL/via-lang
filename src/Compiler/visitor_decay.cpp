// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"
#include "visitor.h"
#include "types.h"
#include "ast.h"

VIA_NAMESPACE_BEGIN

using enum OutputSeverity;

pTypeNode DecayVisitor::visit(AutoNode& auto_node) {
    pTypeNode type = auto_node.expression->infer_type(program);
}

VIA_NAMESPACE_END
