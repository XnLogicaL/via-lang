// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_IR_BUILDER_H_
#define VIA_CORE_IR_BUILDER_H_

#include <via/config.h>
#include <via/types.h>
#include "ir.h"
#include "module/manager.h"
#include "module/module.h"
#include "sema/type_context.h"

namespace via
{

class Module;
class IRBuilder final
{
 public:
  IRBuilder(via::Module* m, const SyntaxTree& ast, DiagContext& diags)
      : mModule(m),
        mAst(ast),
        mAlloc(m->getAllocator()),
        mTypeCtx(m->getManager()->getTypeContext()),
        mDiags(diags)
  {}

 public:
  IRTree build();

 private:
  ir::Expr* lowerExprLit(const ast::ExprLit* exprLit);
  ir::Expr* lowerExprSymbol(const ast::ExprSymbol* exprSym);
  ir::Expr* lowerExprStaticAccess(const ast::ExprStaticAccess* exprStAcc);
  ir::Expr* lowerExprDynamicAccess(const ast::ExprDynAccess* exprDynAcc);
  ir::Expr* lowerExprUnary(const ast::ExprUnary* exprUnary);
  ir::Expr* lowerExprBinary(const ast::ExprBinary* exprBinary);
  ir::Expr* lowerExprGroup(const ast::ExprGroup* exprGroup);
  ir::Expr* lowerExprCall(const ast::ExprCall* exprCall);
  ir::Expr* lowerExprSubscript(const ast::ExprSubscript* exprSubsc);
  ir::Expr* lowerExprCast(const ast::ExprCast* exprCast);
  ir::Expr* lowerExprTernary(const ast::ExprTernary* exprTernary);
  ir::Expr* lowerExprArray(const ast::ExprArray* exprArray);
  ir::Expr* lowerExprTuple(const ast::ExprTuple* exprTuple);
  ir::Expr* lowerExprLambda(const ast::ExprLambda* exprLambda);
  ir::Expr* lowerExpr(const ast::Expr* expr);

 private:
  via::Module* mModule;
  const SyntaxTree& mAst;
  Allocator& mAlloc;
  DiagContext& mDiags;
  sema::TypeContext& mTypeCtx;
};

}  // namespace via

#endif
