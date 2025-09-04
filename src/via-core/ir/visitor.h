// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_IR_VISITOR_H_
#define VIA_CORE_IR_VISITOR_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/visitor.h"

namespace via
{

namespace ir
{

struct TrReturn;
struct TrContinue;
struct TrBreak;
struct TrBranch;
struct TrCondBranch;

struct ExprConstant;
struct ExprSymbol;
struct ExprAccess;
struct ExprUnary;
struct ExprBinary;
struct ExprCall;
struct ExprSubscript;
struct ExprCast;
struct ExprTuple;
struct ExprLambda;

struct StmtVarDecl;
struct StmtFuncDecl;
struct StmtBlock;

class Visitor
{
 public:
  // clang-format off
  virtual void visit(const TrReturn&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(Return)"); }
  virtual void visit(const TrContinue&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(Continue)"); }
  virtual void visit(const TrBreak&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(Break)"); }
  virtual void visit(const TrBranch&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(Br)"); }
  virtual void visit(const TrCondBranch&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(CondBr)"); }

  virtual void visit(const ExprConstant&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(ExprConstant)"); }
  virtual void visit(const ExprSymbol&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(ExprSymbol)"); }
  virtual void visit(const ExprAccess&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(ExprAccess)"); }
  virtual void visit(const ExprUnary&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(ExprUnary)"); }
  virtual void visit(const ExprBinary&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(ExprBinary)"); }
  virtual void visit(const ExprCall&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(ExprCall)"); }
  virtual void visit(const ExprSubscript&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(ExprSubscript)"); }
  virtual void visit(const ExprCast&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(ExprCast)"); }
  virtual void visit(const ExprTuple&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(ExprTuple)"); }
  virtual void visit(const ExprLambda&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(ExprLambda)"); }
  
  virtual void visit(const StmtVarDecl&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(StmtVarDecl)"); }
  virtual void visit(const StmtFuncDecl&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(Function)"); }
  virtual void visit(const StmtBlock&, VisitInfo* vi) { debug::unimplemented("ir::Visitor(Block)"); }
  // clang-format on
};

}  // namespace ir

}  // namespace via

#endif