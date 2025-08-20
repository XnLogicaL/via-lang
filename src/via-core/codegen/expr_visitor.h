// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_EXPR_VISITOR_H_
#define VIA_CORE_EXPR_VISITOR_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
#include "ast/visitor.h"
#include "debug.h"

namespace via {

class Generator;

namespace gen {

class ExprVisitor : public ast::Visitor {
 public:
  ExprVisitor(Generator& ctx) : m_ctx(ctx) {}

 public:
  void visit(const ast::NodeExprLit& elit, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeExprSym& esym, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeExprUn& eun, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeExprBin& ebin, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeExprGroup& egrp, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeExprCall& ecall, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeExprSubs& esubs, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeExprTuple& etup, Box<ast::VisitInfo> vi);
  void visit(const ast::NodeExprLambda& elam, Box<ast::VisitInfo> vi);

  // clang-format off
  void visit(const ast::NodeStmtVar&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeStmtScope&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeStmtIf&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeStmtFor&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeStmtForEach&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeStmtWhile&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeStmtAssign&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeStmtEmpty&, Box<ast::VisitInfo>) { bug("bad visit"); }
  void visit(const ast::NodeStmtExpr&, Box<ast::VisitInfo>) { bug("bad visit"); }
  // clang-format on

 private:
  Generator& m_ctx;
};

}  // namespace gen

}  // namespace via

#endif
