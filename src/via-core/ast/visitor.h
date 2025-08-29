// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_AST_VISITOR_H_
#define VIA_CORE_AST_VISITOR_H_

#include <via/config.h>
#include <via/types.h>
#include "debug.h"

namespace via
{

struct VisitInfo
{
  virtual ~VisitInfo() = default;
};

namespace ast
{

struct ExprLit;
struct ExprSymbol;
struct ExprDynAccess;
struct ExprStaticAccess;
struct ExprUnary;
struct ExprBinary;
struct ExprGroup;
struct ExprCall;
struct ExprSubscript;
struct ExprCast;
struct ExprTernary;
struct ExprArray;
struct ExprTuple;
struct ExprLambda;

struct StmtVarDecl;
struct StmtScope;
struct StmtIf;
struct StmtFor;
struct StmtForEach;
struct StmtWhile;
struct StmtAssign;
struct StmtReturn;
struct StmtEnum;
struct StmtImport;
struct StmtModule;
struct StmtFunctionDecl;
struct StmtStructDecl;
struct StmtTypeDecl;
struct StmtUsing;
struct StmtEmpty;
struct StmtExpr;

struct TypeBuiltin;
struct TypeArray;
struct TypeDict;
struct TypeFunc;

class Visitor
{
 public:
  // clang-format off
  virtual void visit(const ExprLit&, VisitInfo* vi) { debug::unimplemented("visit(ExprLit)"); }
  virtual void visit(const ExprSymbol&, VisitInfo* vi) { debug::unimplemented("visit(ExprSymbol)"); }
  virtual void visit(const ExprDynAccess&, VisitInfo* vi) { debug::unimplemented("visit(ExprDynAccess)"); }
  virtual void visit(const ExprStaticAccess&, VisitInfo* vi) { debug::unimplemented("visit(ExprStaticAccess)"); }
  virtual void visit(const ExprUnary&, VisitInfo* vi) { debug::unimplemented("visit(ExprUnary)"); }
  virtual void visit(const ExprBinary&, VisitInfo* vi) { debug::unimplemented("visit(ExprBinary)"); }
  virtual void visit(const ExprGroup&, VisitInfo* vi) { debug::unimplemented("visit(ExprGroup)"); }
  virtual void visit(const ExprCall&, VisitInfo* vi) { debug::unimplemented("visit(ExprCall)"); }
  virtual void visit(const ExprSubscript&, VisitInfo* vi) { debug::unimplemented("visit(ExprSubscript)"); }
  virtual void visit(const ExprCast&, VisitInfo* vi) { debug::unimplemented("visit(ExprCast)"); }
  virtual void visit(const ExprTernary&, VisitInfo* vi) { debug::unimplemented("visit(ExprTernary)"); }
  virtual void visit(const ExprArray&, VisitInfo* vi) { debug::unimplemented("visit(ExprArray)"); }
  virtual void visit(const ExprTuple&, VisitInfo* vi) { debug::unimplemented("visit(ExprTuple)"); }
  virtual void visit(const ExprLambda&, VisitInfo* vi) { debug::unimplemented("visit(ExprLambda)"); }

  virtual void visit(const StmtVarDecl&, VisitInfo* vi) { debug::unimplemented("visit(StmtVarDecl)"); }
  virtual void visit(const StmtScope&, VisitInfo* vi) { debug::unimplemented("visit(StmtScope)"); }
  virtual void visit(const StmtIf&, VisitInfo* vi) { debug::unimplemented("visit(StmtIf)"); }
  virtual void visit(const StmtFor&, VisitInfo* vi) { debug::unimplemented("visit(StmtFor)"); }
  virtual void visit(const StmtForEach&, VisitInfo* vi) { debug::unimplemented("visit(StmtForEach)"); }
  virtual void visit(const StmtWhile&, VisitInfo* vi) { debug::unimplemented("visit(StmtWhile)"); }
  virtual void visit(const StmtAssign&, VisitInfo* vi) { debug::unimplemented("visit(StmtAssign)"); }
  virtual void visit(const StmtReturn&, VisitInfo* vi) { debug::unimplemented("visit(StmtReturn)"); }
  virtual void visit(const StmtEnum&, VisitInfo* vi) { debug::unimplemented("visit(StmtEnum)"); }
  virtual void visit(const StmtImport&, VisitInfo* vi) { debug::unimplemented("visit(StmtImport)"); }
  virtual void visit(const StmtModule&, VisitInfo* vi) { debug::unimplemented("visit(StmtModule)"); }
  virtual void visit(const StmtFunctionDecl&, VisitInfo* vi) { debug::unimplemented("visit(StmtFunctionDecl)"); }
  virtual void visit(const StmtStructDecl&, VisitInfo* vi) { debug::unimplemented("visit(StmtStructDecl)"); }
  virtual void visit(const StmtTypeDecl&, VisitInfo* vi) { debug::unimplemented("visit(StmtTypeDecl)"); }
  virtual void visit(const StmtUsing&, VisitInfo* vi) { debug::unimplemented("visit(StmtUsing)"); }
  virtual void visit(const StmtEmpty&, VisitInfo* vi) { debug::unimplemented("visit(StmtEmpty)"); }
  virtual void visit(const StmtExpr&, VisitInfo* vi) { debug::unimplemented("visit(StmtExpr)"); }

  virtual void visit(const TypeBuiltin&, VisitInfo* vi) { debug::unimplemented("visit(TypeBuiltin)"); }
  virtual void visit(const TypeArray&, VisitInfo* vi) { debug::unimplemented("visit(TypeArray)"); }
  virtual void visit(const TypeDict&, VisitInfo* vi) { debug::unimplemented("visit(TypeDict)"); }
  virtual void visit(const TypeFunc&, VisitInfo* vi) { debug::unimplemented("visit(TypeFunc)"); }
  // clang-format on
};

}  // namespace ast

}  // namespace via

#endif
