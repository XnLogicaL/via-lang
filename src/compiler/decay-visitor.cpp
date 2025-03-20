// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"
#include "visitor.h"
#include "compiler-types.h"
#include "ast.h"

VIA_NAMESPACE_BEGIN

using enum OutputSeverity;

pTypeNode DecayVisitor::visit(AutoNode& auto_node) {
    return auto_node.expression->infer_type(program);
}

pTypeNode DecayVisitor::visit(GenericNode&) {
    return nullptr;
}

pTypeNode DecayVisitor::visit(UnionNode&) {
    return nullptr;
}

pTypeNode DecayVisitor::visit(FunctionTypeNode&) {
    return nullptr;
}

VIA_NAMESPACE_END
