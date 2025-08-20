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
  StmtVisitor(Generator& ctx) : m_ctx(ctx), m_vis(ctx) {}

 public:
  // clang-format off
  void visit(const ast::NodeExprLit&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeExprSym&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeExprUn&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeExprBin&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeExprGroup&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeExprCall&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeExprSubs&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeExprTuple&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeExprLambda&, Box<ast::VisitInfo>) { bug("bad visit"); }
  // clang-format on

  void visit(const ast::NodeStmtVar& svar, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeStmtScope& sscp, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeStmtIf& sif, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeStmtFor& sfor, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeStmtForEach& sfeach, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeStmtWhile& swhl, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeStmtAssign& sasgn, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeStmtEmpty& semt, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeStmtExpr& sexpr, Box<ast::VisitInfo> vi);

 private:
  Generator& m_ctx;
  ExprVisitor m_vis;
};

}  // namespace gen

}  // namespace via

#endif
