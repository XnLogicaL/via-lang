// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_PARSER_OPTIMIZE_FOLD_H_
#define VIA_PARSER_OPTIMIZE_FOLD_H_

#include <via/config.h>
#include "optimize.h"

namespace via {

namespace core {

namespace parser {

class FoldOptimizationPass final : public OptimizationPass {
 public:
  using OptimizationPass::OptimizationPass;
  void apply(AstBuf& ast) const override;

 private:
  using OptimizationPass::alloc;
  ast::ExprNode* apply_expr(const ast::ExprNode* expr);
};

}  // namespace parser

}  // namespace core

}  // namespace via

#endif
