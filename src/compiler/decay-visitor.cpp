// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "stack.h"
#include "visitor.h"
#include "compiler-types.h"
#include "ast.h"

namespace via {

p_type_node_t decay_node_visitor::visit(auto_type_node& auto_node) {
  return auto_node.expression->infer_type(unit_ctx);
}

p_type_node_t decay_node_visitor::visit(generic_type_node&) {
  return nullptr;
}

p_type_node_t decay_node_visitor::visit(union_type_node&) {
  return nullptr;
}

p_type_node_t decay_node_visitor::visit(FunctionTypeNode&) {
  return nullptr;
}

} // namespace via
