// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "optimize_fold.h"
#include <algorithm>
#include <ranges>

namespace via {

namespace core {

namespace parser {

using lex::TokenKind;
using namespace ast;

void FoldOptimizationPass::apply(AstBuf& ast) {
  for (usize pos = 0; pos < ast.size(); pos++) {
    apply_stmt(ast, pos, ast[pos]);
    ++pos;
  }
}

void FoldOptimizationPass::apply_if(AstBuf& ast, usize pos, NodeStmtIf* node) {
  for (usize j = 0; j < node->brs.size(); j++) {
    const auto& br = node->brs[j];

    if (auto psv = apply_expr(br.cnd)) {  // constexpr condition
      if (psv->as_cbool()) {              // always truthy condition
        ast[pos] = br.br;
        break;
      } else {  // always falsy condition
        node->brs.erase(node->brs.begin() + j);
      }
    }

    ++j;
  }
}

void FoldOptimizationPass::apply_stmt(AstBuf& ast, usize pos, StmtNode* node) {
#define CASE(T, func)                      \
  if TRY_COERCE (T, _subclass_##T, node) { \
    func(ast, pos, _subclass_##T);         \
    return;                                \
  }

  CASE(NodeStmtIf, apply_if);
#undef CASE
}

PseudoValue* FoldOptimizationPass::apply_un(const NodeExprUn* un) {
  switch (un->op->kind) {
    case TokenKind::MINUS:
      if (auto psv = apply_expr(un->expr)) {
        if (psv->kind == PseudoValue::Int) {
          psv->u.i = -psv->u.i;
          return psv;
        } else if (psv->kind == PseudoValue::Float) {
          psv->u.fp = -psv->u.fp;
          return psv;
        }
      }
      break;
    default:
      break;
  }

  return NULL;
}

PseudoValue* FoldOptimizationPass::apply_bin(const NodeExprBin* bin) {
  return NULL;
}

}  // namespace parser

}  // namespace core

}  // namespace via
