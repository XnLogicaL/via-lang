// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_PARSER_OPTIMIZE_FOLD_H_
#define VIA_PARSER_OPTIMIZE_FOLD_H_

#include <via/config.h>
#include "ast.h"
#include "lexer/token.h"
#include "optimize.h"
#include "sema/constexpr.h"

namespace via {

namespace core {

namespace parser {

class FoldOptimizationPass final : public OptimizationPass {
 public:
  using OptimizationPass::OptimizationPass;

  void apply(AstBuf& ast) override;

 private:
  using OptimizationPass::alloc;

  void apply_if(AstBuf& ast, usize pos, ast::NodeStmtIf* node);
  void apply_stmt(AstBuf& ast, usize pos, ast::StmtNode* node);

  PseudoValue* apply_un(const ast::NodeExprUn* un);
  PseudoValue* apply_bin(const ast::NodeExprBin* bin);
  PseudoValue* apply_expr(const ast::ExprNode* expr);
};

}  // namespace parser

}  // namespace core

}  // namespace via

#endif
