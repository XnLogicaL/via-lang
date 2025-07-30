// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "optimize_fold.h"

namespace via {

namespace core {

namespace parser {

using namespace ast;

void FoldOptimizationPass::apply(AstBuf& ast) const {
  for (const StmtNode* ptr : ast) {
    if TRY_COERCE (NodeStmtIf, ifs, ptr)
      continue;
  }
}

}  // namespace parser

}  // namespace core

}  // namespace via
