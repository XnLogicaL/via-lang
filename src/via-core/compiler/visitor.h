// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_VISITOR_H_
#define VIA_CORE_VISITOR_H_

#include <via/config.h>
#include <via/types.h>
#include "diagnostics.h"
#include "parser/ast.h"
#include "sema/context.h"

namespace via {

class ASTVisitor {
 public:
  virtual void visit(const ast::NodeExprLit& elit, u16 dst) = 0;
  virtual void visit(const ast::NodeExprSym& esym, u16 dst) = 0;
  virtual void visit(const ast::NodeExprUn& eun, u16 dst) = 0;
  virtual void visit(const ast::NodeExprBin& ebin, u16 dst) = 0;
  virtual void visit(const ast::NodeExprGroup& egrp, u16 dst) = 0;
  virtual void visit(const ast::NodeExprCall& ecall, u16 dst) = 0;
  virtual void visit(const ast::NodeExprSubs& esubs, u16 dst) = 0;
  virtual void visit(const ast::NodeExprTuple& etup, u16 dst) = 0;
  virtual void visit(const ast::NodeExprLambda& elam, u16 dst) = 0;

  virtual void visit(const ast::NodeStmtVar& svar) = 0;
  virtual void visit(const ast::NodeStmtScope& svar) = 0;
  virtual void visit(const ast::NodeStmtIf& svar) = 0;
  virtual void visit(const ast::NodeStmtFor& svar) = 0;
  virtual void visit(const ast::NodeStmtForEach& svar) = 0;
  virtual void visit(const ast::NodeStmtWhile& svar) = 0;
  virtual void visit(const ast::NodeStmtAssign& svar) = 0;
  virtual void visit(const ast::NodeStmtEmpty& svar) = 0;
  virtual void visit(const ast::NodeStmtExpr& svar) = 0;
};

class TranslationVisitor : public ASTVisitor {
 public:
  TranslationVisitor(Diagnostics& diag) : diag(diag) {}

 protected:
  Diagnostics& diag;
  sema::SemaContext sema;
};

class ExprVisitor : public TranslationVisitor {
 public:
  using TranslationVisitor::TranslationVisitor;

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
};

class StmtVisitor : public TranslationVisitor {
 public:
  using TranslationVisitor::TranslationVisitor;

 public:
  void visit(const ast::NodeStmtVar& svar) override;
  void visit(const ast::NodeStmtScope& svar) override;
  void visit(const ast::NodeStmtIf& svar) override;
  void visit(const ast::NodeStmtFor& svar) override;
  void visit(const ast::NodeStmtForEach& svar) override;
  void visit(const ast::NodeStmtWhile& svar) override;
  void visit(const ast::NodeStmtAssign& svar) override;
  void visit(const ast::NodeStmtEmpty& svar) override;
  void visit(const ast::NodeStmtExpr& svar) override;
};

}  // namespace via

#endif
