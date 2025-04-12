//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#include "stack.h"
#include "visitor.h"
#include "compiler-types.h"
#include "ast.h"

namespace via {

TypeNodeBase* DecayNodeVisitor::visit(AutoTypeNode& auto_node) {
  return auto_node.expression->infer_type(ctx.unit_ctx);
}

TypeNodeBase* DecayNodeVisitor::visit(GenericTypeNode&) {
  return nullptr;
}

TypeNodeBase* DecayNodeVisitor::visit(UnionTypeNode&) {
  return nullptr;
}

TypeNodeBase* DecayNodeVisitor::visit(FunctionTypeNode&) {
  return nullptr;
}

TypeNodeBase* DecayNodeVisitor::visit(ArrayTypeNode&) {
  return nullptr;
}

} // namespace via
