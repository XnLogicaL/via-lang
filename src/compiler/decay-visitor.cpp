// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"
#include "visitor.h"
#include "compiler-types.h"
#include "ast.h"

VIA_NAMESPACE_BEGIN

pTypeNode DecayVisitor::visit(AutoTypeNode& auto_node) {
  return auto_node.expression->infer_type(unit_ctx);
}

pTypeNode DecayVisitor::visit(GenericTypeNode&) {
  return nullptr;
}

pTypeNode DecayVisitor::visit(UnionTypeNode&) {
  return nullptr;
}

pTypeNode DecayVisitor::visit(FunctionTypeNode&) {
  return nullptr;
}

VIA_NAMESPACE_END
