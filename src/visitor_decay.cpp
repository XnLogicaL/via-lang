// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "visitor.h"

namespace via {

TypeNode* DecayNodeVisitor::visit(AutoTypeNode& auto_node) {
  return auto_node.expression->infer_type(ctx.lctx);
}

TypeNode* DecayNodeVisitor::visit(NodeGenType&) {
  return nullptr;
}

TypeNode* DecayNodeVisitor::visit(NodeUnionType&) {
  return nullptr;
}

TypeNode* DecayNodeVisitor::visit(NodeFuncType&) {
  return nullptr;
}

TypeNode* DecayNodeVisitor::visit(NodeArrType&) {
  return nullptr;
}

} // namespace via
