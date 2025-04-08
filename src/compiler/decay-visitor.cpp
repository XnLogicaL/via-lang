//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#include "stack.h"
#include "visitor.h"
#include "compiler-types.h"
#include "ast.h"

namespace via {

TypeNodeBase* decay_node_visitor::visit(AutoTypeNode& auto_node) {
  return auto_node.expression->infer_type(unit_ctx);
}

TypeNodeBase* decay_node_visitor::visit(GenericTypeNode&) {
  return nullptr;
}

TypeNodeBase* decay_node_visitor::visit(UnionTypeNode&) {
  return nullptr;
}

TypeNodeBase* decay_node_visitor::visit(FunctionTypeNode&) {
  return nullptr;
}

} // namespace via
