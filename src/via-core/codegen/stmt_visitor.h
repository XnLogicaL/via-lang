// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_STMT_VISITOR_H_
#define VIA_CORE_STMT_VISITOR_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
#include "ast/visitor.h"
#include "expr_visitor.h"

namespace via {

class Generator;

namespace gen {

class StmtVisitor : public ast::Visitor {
 public:
  StmtVisitor(Generator& ctx) : ctx(ctx), expr_vis(ctx) {}

 public:
  void visit(const ast::NodeStmtVar& svar) override;
  void visit(const ast::NodeStmtScope& sscp) override;
  void visit(const ast::NodeStmtIf& sif) override;
  void visit(const ast::NodeStmtFor& sfor) override;
  void visit(const ast::NodeStmtForEach& sfeach) override;
  void visit(const ast::NodeStmtWhile& swhl) override;
  void visit(const ast::NodeStmtAssign& sasgn) override;
  void visit(const ast::NodeStmtEmpty& semt) override;
  void visit(const ast::NodeStmtExpr& sexpr) override;

 private:
  Generator& ctx;
  ExprVisitor expr_vis;
};

}  // namespace gen

}  // namespace via

#endif
