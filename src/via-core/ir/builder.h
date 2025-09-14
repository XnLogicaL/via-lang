/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "ir.h"
#include "module/manager.h"
#include "module/module.h"
#include "sema/stack.h"
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
        mSymbolTable(m->getManager()->getSymbolTable()),
        mDiags(diags)
  {}

 public:
  IRTree build();

 private:
  const sema::Type* typeOf(const ast::Expr* expr) noexcept;
  const sema::Type* typeOf(const ast::Type* type) noexcept;

  inline ir::StmtBlock* endBlock() noexcept
  {
    mShouldPushBlock = true;
    return mCurrentBlock;
  }

  inline ir::StmtBlock* newBlock(SymbolId symbol) noexcept
  {
    ir::StmtBlock* block = mCurrentBlock;
    mShouldPushBlock = false;
    mCurrentBlock = mAlloc.emplace<ir::StmtBlock>();
    mCurrentBlock->name = symbol;
    return block;
  }

  // clang-format off
  auto internSymbol(std::string symbol) { return mSymbolTable.intern(symbol); }
  auto internSymbol(const via::Token& symbol) { return mSymbolTable.intern(symbol.toString()); }
  auto nextLabelSymbol() { return internSymbol(std::format(".LB{}", iota<usize>())); }

  const ir::Expr* lowerExprLit(const ast::ExprLit* exprLit);
  const ir::Expr* lowerExprSymbol(const ast::ExprSymbol* exprSym);
  const ir::Expr* lowerExprStaticAccess(const ast::ExprStaticAccess* exprStAcc);
  const ir::Expr* lowerExprDynamicAccess(const ast::ExprDynAccess* exprDynAcc);
  const ir::Expr* lowerExprUnary(const ast::ExprUnary* exprUnary);
  const ir::Expr* lowerExprBinary(const ast::ExprBinary* exprBinary);
  const ir::Expr* lowerExprGroup(const ast::ExprGroup* exprGroup);
  const ir::Expr* lowerExprCall(const ast::ExprCall* exprCall);
  const ir::Expr* lowerExprSubscript(const ast::ExprSubscript* exprSubsc);
  const ir::Expr* lowerExprCast(const ast::ExprCast* exprCast);
  const ir::Expr* lowerExprTernary(const ast::ExprTernary* exprTernary);
  const ir::Expr* lowerExprArray(const ast::ExprArray* exprArray);
  const ir::Expr* lowerExprTuple(const ast::ExprTuple* exprTuple);
  const ir::Expr* lowerExprLambda(const ast::ExprLambda* exprLambda);
  const ir::Expr* lowerExpr(const ast::Expr* expr);

  const ir::Stmt* lowerStmtVarDecl(const ast::StmtVarDecl* stmtVarDecl);
  const ir::Stmt* lowerStmtScope(const ast::StmtScope* stmtScope);
  const ir::Stmt* lowerStmtIf(const ast::StmtIf* stmtIf);
  const ir::Stmt* lowerStmtFor(const ast::StmtFor* stmtFor);
  const ir::Stmt* lowerStmtForEach(const ast::StmtForEach* StmtForEach);
  const ir::Stmt* lowerStmtWhile(const ast::StmtWhile* StmtWhile);
  const ir::Stmt* lowerStmtAssign(const ast::StmtAssign* StmtAssign);
  const ir::Stmt* lowerStmtReturn(const ast::StmtReturn* stmtReturn);
  const ir::Stmt* lowerStmtEnum(const ast::StmtEnum* stmtEnum);
  const ir::Stmt* lowerStmtModule(const ast::StmtModule* stmtModule);
  const ir::Stmt* lowerStmtImport(const ast::StmtImport* stmtImport);
  const ir::Stmt* lowerStmtFunctionDecl(const ast::StmtFunctionDecl* stmtFunctionDecl);
  const ir::Stmt* lowerStmtStructDecl(const ast::StmtStructDecl* stmtStructDecl);
  const ir::Stmt* lowerStmtTypeDecl(const ast::StmtTypeDecl* stmtTypeDecl);
  const ir::Stmt* lowerStmtUsing(const ast::StmtUsing* stmtUsing);
  const ir::Stmt* lowerStmtExpr(const ast::StmtExpr* stmtExpr);
  const ir::Stmt* lowerStmt(const ast::Stmt* stmt);
  // clang-format on

 private:
  via::Module* mModule;
  const SyntaxTree& mAst;
  Allocator& mAlloc;
  DiagContext& mDiags;
  sema::StackState mStack;
  sema::TypeContext& mTypeCtx;
  SymbolTable& mSymbolTable;
  bool mShouldPushBlock;
  ir::StmtBlock* mCurrentBlock;
};

}  // namespace via
