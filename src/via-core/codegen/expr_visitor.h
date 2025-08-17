// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_EXPR_VISITOR_H_
#define VIA_CORE_EXPR_VISITOR_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
#include "ast/visitor.h"

namespace via {

class Generator;

namespace gen {

class ExprVisitor : public ast::Visitor {
 public:
  ExprVisitor(Generator& ctx) : ctx(ctx) {}

 public:
  void visit(const ast::NodeExprLit& elit, u16 dst) override;
  void visit(const ast::NodeExprSym& esym, u16 dst) override;
  void visit(const ast::NodeExprUn& eun, u16 dst) override;
  void visit(const ast::NodeExprBin& ebin, u16 dst) override;
  void visit(const ast::NodeExprGroup& egrp, u16 dst) override;
  void visit(const ast::NodeExprCall& ecall, u16 dst) override;
  void visit(const ast::NodeExprSubs& esubs, u16 dst) override;
  void visit(const ast::NodeExprTuple& etup, u16 dst) override;
  void visit(const ast::NodeExprLambda& elam, u16 dst) override;

 private:
  Generator& ctx;
};

}  // namespace gen

}  // namespace via

#endif
