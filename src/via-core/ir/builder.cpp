/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "builder.h"
#include "ir/ir.h"
#include "sema/stack.h"
#include "utility.h"

namespace ir = via::ir;
namespace sema = via::sema;

using Dk = via::Diagnosis::Kind;
using Ak = ir::ExprAccess::Kind;
using Btk = sema::BuiltinType::Kind;

#define PMR_CASE(NODE, TYPE, HANDLER)     \
  if TRY_COERCE (const TYPE, inner, NODE) \
    return HANDLER(inner);

via::SymbolTable& symbolTable = via::SymbolTable::instance();

static via::SymbolId internSymbol(const std::string& symbol)
{
  return via::SymbolTable::instance().intern(symbol);
}

static via::SymbolId internSymbol(const via::Token& symbol)
{
  return via::SymbolTable::instance().intern(symbol.toString());
}

const sema::Type* via::IRBuilder::typeOf(const ast::Expr* expr) noexcept
{
  using enum Token::Kind;
  using enum sema::BuiltinType::Kind;

  if TRY_COERCE (const ast::ExprLit, lit, expr) {
    sema::BuiltinType::Kind kind;

    switch (lit->tok->kind) {
      case LIT_NIL:
        kind = Nil;
        break;
      case LIT_TRUE:
      case LIT_FALSE:
        kind = Bool;
        break;
      case LIT_INT:
      case LIT_XINT:
      case LIT_BINT:
        kind = Int;
        break;
      case LIT_FLOAT:
        kind = Float;
        break;
      case LIT_STRING:
        kind = String;
        break;
      default:
        debug::bug("unhandled literal expression");
    }

    return mAlloc.emplace<sema::BuiltinType>(kind);
  } else if TRY_COERCE (const ast::ExprSymbol, sym, expr) {
    sema::Frame& frame = mStack.top();

    // Local variable
    if (auto local = frame.getLocal(sym->sym->toStringView())) {
      return local->local.getType();
    }

    debug::todo("check other kinds of symbols");
  } else if TRY_COERCE (const ast::ExprDynAccess, dyna, expr) {
    return nullptr;
  } else if TRY_COERCE (const ast::ExprStaticAccess, sta, expr) {
    return nullptr;
  } else if TRY_COERCE (const ast::ExprUnary, unary, expr) {
    UnaryOp op = toUnaryOp(unary->op->kind);
    auto* type = typeOf(unary->expr);

    switch (op) {
      case UnaryOp::REF:
        if (ast::isLValue(unary->expr)) {
          return type;
        } else {
          mDiags.report<Dk::Error>(
            unary->expr->loc,
            "Invalid unary operation '&' (REF) on a non-lvalue expression");
        }
        break;
      case UnaryOp::NEG: {
        if (type->isArithmetic()) {
          return type;
        } else {
          mDiags.report<Dk::Error>(
            unary->expr->loc,
            std::format("Invalid unary expression '-' (NEG) on non-arithmetic "
                        "expression of type '{}'",
                        type->dump()));
        }
      } break;
      case UnaryOp::NOT:
        return mTypeCtx.getBuiltinTypeInstance(Btk::Bool);
      case UnaryOp::BNOT: {
        if (type->isIntegral()) {
        } else {
          mDiags.report<Dk::Error>(
            unary->expr->loc,
            std::format("Invalid unary expression '~' (BNOT) on non-integral "
                        "expression of type '{}'",
                        type->dump()));
        }
      } break;
    }

    return nullptr;
  }

  debug::bug("unhandled typeOf(expr)");
}

const ir::Expr* via::IRBuilder::lowerExprLit(const ast::ExprLit* exprLit)
{
  auto* constant = mAlloc.emplace<ir::ExprConstant>();
  constant->value = *sema::ConstValue::fromToken(*exprLit->tok);
  constant->type = typeOf(exprLit);
  return constant;
}

const ir::Expr* via::IRBuilder::lowerExprSymbol(const ast::ExprSymbol* exprSym)
{
  std::string symbol = exprSym->sym->toString();
  sema::Frame& frame = mStack.top();

  auto* sym = mAlloc.emplace<ir::ExprSymbol>();
  sym->symbol = symbolTable.intern(symbol);

  // Local-level symbol
  if (auto local = frame.getLocal(symbol)) {
    sym->type = local->local.getType();
    return sym;
  }

  mDiags.report<Dk::Error>(exprSym->loc,
                           std::format("Use of undefined symbol '{}'", symbol));
  return nullptr;
}

const ir::Expr* via::IRBuilder::lowerExprStaticAccess(
  const ast::ExprStaticAccess* exprStAcc)
{
  // Check for module thing
  if TRY_COERCE (const ast::ExprSymbol, rootSymbol, exprStAcc->root) {
    std::string symbol = rootSymbol->sym->toString();
    ModuleManager* manager = mModule->getManager();

    if (Module* module = manager->getModuleByName(symbol)) {
      SymbolId lowSymbolId = internSymbol(exprStAcc->index->toString());
      if (auto def = module->lookup(lowSymbolId)) {
        auto ident = def.getValue()->getIdentity();
        auto finalSymbol = std::format(
          "module<{}>::{}", symbol,
          SymbolTable::instance().lookup(ident).valueOr("<unknown>"));

        auto* access = mAlloc.emplace<ir::ExprSymbol>();
        access->symbol = internSymbol(finalSymbol);
        access->type = nullptr; /* TODO */
        return access;
      }
    }
  }

  auto* access = mAlloc.emplace<ir::ExprAccess>();
  access->kind = Ak::STATIC;
  access->root = lowerExpr(exprStAcc->root);
  access->index = internSymbol(*exprStAcc->index);
  access->type = typeOf(exprStAcc);
  return access;
}

const ir::Expr* via::IRBuilder::lowerExprDynamicAccess(
  const ast::ExprDynAccess* exprDynAcc)
{
  auto* access = mAlloc.emplace<ir::ExprAccess>();
  access->kind = Ak::DYNAMIC;
  access->root = lowerExpr(exprDynAcc->root);
  access->index = internSymbol(*exprDynAcc->index);
  access->type = typeOf(exprDynAcc);
  return access;
}

const ir::Expr* via::IRBuilder::lowerExprUnary(const ast::ExprUnary* exprUnary)
{
  auto* unary = mAlloc.emplace<ir::ExprUnary>();
  unary->op = toUnaryOp(exprUnary->op->kind);
  unary->type = typeOf(exprUnary);
  unary->expr = lowerExpr(exprUnary->expr);
  return unary;
}

const ir::Expr* via::IRBuilder::lowerExprBinary(
  const ast::ExprBinary* exprBinary)
{
  auto* binary = mAlloc.emplace<ir::ExprBinary>();
  binary->op = toBinaryOp(exprBinary->op->kind);
  binary->lhs = lowerExpr(exprBinary->lhs);
  binary->rhs = lowerExpr(exprBinary->rhs);
  binary->type = typeOf(exprBinary);
  return binary;
}

const ir::Expr* via::IRBuilder::lowerExprGroup(const ast::ExprGroup* exprGroup)
{
  return lowerExpr(exprGroup->expr);
}

const ir::Expr* via::IRBuilder::lowerExprCall(const ast::ExprCall* exprCall)
{
  auto* call = mAlloc.emplace<ir::ExprCall>();
  call->callee = lowerExpr(exprCall->lval);
  call->type = nullptr; /* TODO: Type of this expression should be the return
                           type of the callee function. */
  call->args = [&]() {
    Vec<const ir::Expr*> args;
    for (const auto& astArg : exprCall->args) {
      args.push_back(lowerExpr(astArg));
    }
    return args;
  }();
  return call;
}

const ir::Expr* via::IRBuilder::lowerExprSubscript(
  const ast::ExprSubscript* exprSubsc)
{
  return nullptr;
}

const ir::Expr* via::IRBuilder::lowerExprCast(const ast::ExprCast* exprCast)
{
  return nullptr;
}

const ir::Expr* via::IRBuilder::lowerExprTernary(
  const ast::ExprTernary* exprTernary)
{
  return nullptr;
}

const ir::Expr* via::IRBuilder::lowerExprArray(const ast::ExprArray* exprArray)
{
  return nullptr;
}

const ir::Expr* via::IRBuilder::lowerExprTuple(const ast::ExprTuple* exprTuple)
{
  return nullptr;
}

const ir::Expr* via::IRBuilder::lowerExprLambda(
  const ast::ExprLambda* exprLambda)
{
  return nullptr;
}

const ir::Expr* via::IRBuilder::lowerExpr(const ast::Expr* expr)
{
  PMR_CASE(expr, ast::ExprLit, lowerExprLit)
  PMR_CASE(expr, ast::ExprSymbol, lowerExprSymbol)
  PMR_CASE(expr, ast::ExprStaticAccess, lowerExprStaticAccess)
  PMR_CASE(expr, ast::ExprDynAccess, lowerExprDynamicAccess)
  PMR_CASE(expr, ast::ExprUnary, lowerExprUnary)
  PMR_CASE(expr, ast::ExprBinary, lowerExprBinary)
  PMR_CASE(expr, ast::ExprGroup, lowerExprGroup)
  PMR_CASE(expr, ast::ExprCall, lowerExprCall)
  PMR_CASE(expr, ast::ExprSubscript, lowerExprSubscript)
  PMR_CASE(expr, ast::ExprCast, lowerExprCast)
  PMR_CASE(expr, ast::ExprTernary, lowerExprTernary)
  PMR_CASE(expr, ast::ExprArray, lowerExprArray)
  PMR_CASE(expr, ast::ExprTuple, lowerExprTuple)
  PMR_CASE(expr, ast::ExprLambda, lowerExprLambda)

  debug::bug(
    std::format("unhandled case IRBuilder::lowerExpr({})", TYPENAME(*expr)));
}

const ir::Stmt* via::IRBuilder::lowerStmtVarDecl(
  const ast::StmtVarDecl* stmtVarDecl)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtScope(const ast::StmtScope* stmtScope)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtIf(const ast::StmtIf* stmtIf)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtFor(const ast::StmtFor* stmtFor)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtForEach(
  const ast::StmtForEach* StmtForEach)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtWhile(const ast::StmtWhile* StmtWhile)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtAssign(
  const ast::StmtAssign* StmtAssign)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtReturn(
  const ast::StmtReturn* stmtReturn)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtEnum(const ast::StmtEnum* stmtEnum)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtModule(
  const ast::StmtModule* stmtModule)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtImport(
  const ast::StmtImport* stmtImport)
{
  QualName path;

  for (const Token* tok : stmtImport->path) {
    path.push_back(tok->toString());
  }

  if (auto result = mModule->resolveImport(path); result.hasError()) {
    mDiags.report<Dk::Error>(stmtImport->loc, result.takeError().toString());
  }

  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtFunctionDecl(
  const ast::StmtFunctionDecl* stmtFunctionDecl)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtStructDecl(
  const ast::StmtStructDecl* stmtStructDecl)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtTypeDecl(
  const ast::StmtTypeDecl* stmtTypeDecl)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtUsing(const ast::StmtUsing* stmtUsing)
{
  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtExpr(const ast::StmtExpr* stmtExpr)
{
  auto* expr = mAlloc.emplace<ir::StmtExpr>();
  expr->expr = lowerExpr(stmtExpr->expr);
  return expr;
}

const ir::Stmt* via::IRBuilder::lowerStmt(const ast::Stmt* stmt)
{
  PMR_CASE(stmt, ast::StmtVarDecl, lowerStmtVarDecl);
  PMR_CASE(stmt, ast::StmtScope, lowerStmtScope);
  PMR_CASE(stmt, ast::StmtIf, lowerStmtIf);
  PMR_CASE(stmt, ast::StmtFor, lowerStmtFor);
  PMR_CASE(stmt, ast::StmtForEach, lowerStmtForEach);
  PMR_CASE(stmt, ast::StmtWhile, lowerStmtWhile);
  PMR_CASE(stmt, ast::StmtAssign, lowerStmtAssign);
  PMR_CASE(stmt, ast::StmtReturn, lowerStmtReturn);
  PMR_CASE(stmt, ast::StmtEnum, lowerStmtEnum);
  PMR_CASE(stmt, ast::StmtModule, lowerStmtModule);
  PMR_CASE(stmt, ast::StmtImport, lowerStmtImport);
  PMR_CASE(stmt, ast::StmtFunctionDecl, lowerStmtFunctionDecl);
  PMR_CASE(stmt, ast::StmtStructDecl, lowerStmtStructDecl);
  PMR_CASE(stmt, ast::StmtTypeDecl, lowerStmtTypeDecl);
  PMR_CASE(stmt, ast::StmtUsing, lowerStmtUsing);
  PMR_CASE(stmt, ast::StmtExpr, lowerStmtExpr);

  if TRY_COERCE (const ast::StmtEmpty, _, stmt)
    return nullptr;

  debug::bug(
    std::format("unhandled case IRBuilder::lowerStmt({})", TYPENAME(*stmt)));
}

via::IRTree via::IRBuilder::build()
{
  // Push root stack frame
  mStack.push({});

  IRTree tree;

  for (const auto& astStmt : mAst) {
    if (const ir::Stmt* loweredStmt = lowerStmt(astStmt)) {
      tree.push_back(loweredStmt);
    }
  }

  return tree;
}
