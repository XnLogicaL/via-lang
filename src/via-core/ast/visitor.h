// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_VISITOR_H_
#define VIA_CORE_VISITOR_H_

#include <via/config.h>
#include <via/types.h>
#include "panic.h"

namespace via {

namespace ast {

struct NodeExprLit;
struct NodeExprSym;
struct NodeExprUn;
struct NodeExprBin;
struct NodeExprGroup;
struct NodeExprCall;
struct NodeExprSubs;
struct NodeExprTuple;
struct NodeExprLambda;

struct NodeStmtVar;
struct NodeStmtScope;
struct NodeStmtIf;
struct NodeStmtFor;
struct NodeStmtForEach;
struct NodeStmtWhile;
struct NodeStmtAssign;
struct NodeStmtEmpty;
struct NodeStmtExpr;

class Visitor {
 public:
  // clang-format off
  virtual void visit(const NodeExprLit& elit, u16 dst) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeExprSym& esym, u16 dst) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeExprUn& eun, u16 dst) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeExprBin& ebin, u16 dst) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeExprGroup& egrp, u16 dst) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeExprCall& ecall, u16 dst) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeExprSubs& esubs, u16 dst) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeExprTuple& etup, u16 dst) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeExprLambda& elam, u16 dst) { VIA_UNIMPLEMENTED(); }

  virtual void visit(const NodeStmtVar& svar) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeStmtScope& sscp) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeStmtIf& sif) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeStmtFor& sfor) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeStmtForEach& sfeach) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeStmtWhile& swhl) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeStmtAssign& sasgn) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeStmtEmpty& semt) { VIA_UNIMPLEMENTED(); }
  virtual void visit(const NodeStmtExpr& sexpr) { VIA_UNIMPLEMENTED(); }
  // clang-format on
};

}  // namespace ast

}  // namespace via

#endif
