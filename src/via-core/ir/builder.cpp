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

namespace ast = via::ast;
namespace ir = via::ir;
namespace sema = via::sema;

using Dk = via::Diagnosis::Kind;
using Ak = ir::ExprAccess::Kind;
using Btk = sema::BuiltinType::Kind;

via::SymbolTable& symbolTable = via::SymbolTable::instance();

static via::SymbolId internSymbol(const std::string& symbol)
{
  return via::SymbolTable::instance().intern(symbol);
}

static via::SymbolId internSymbol(const via::Token& symbol)
{
  return via::SymbolTable::instance().intern(symbol.toString());
}

ir::Expr* via::IRBuilder::lowerExprLit(const ast::ExprLit* exprLit)
{
  auto* constant = mAlloc.emplace<ir::ExprConstant>();
  constant->value = *sema::ConstValue::fromToken(*exprLit->tok);
  constant->type = *sema::Type::infer(mAlloc, exprLit);
  return constant;
}

ir::Expr* via::IRBuilder::lowerExprSymbol(const ast::ExprSymbol* exprSym)
{
  std::string symbol = exprSym->sym->toString();
  sema::Frame& frame = sema::stack::top();

  if (auto local = frame.getLocal(symbol)) {
    auto* sym = mAlloc.emplace<ir::ExprSymbol>();
    sym->symbol = symbolTable.intern(symbol);
    sym->type = local->local.getType();
    sym->local = &local.getValue();
    return sym;
  } else {
    mDiags.report<Dk::Error>(
      exprSym->loc, fmt::format("Use of undefined symbol '{}'", symbol));
    return nullptr;
  }
}

ir::Expr* via::IRBuilder::lowerExprStaticAccess(
  const ast::ExprStaticAccess* exprStAcc)
{
  auto* access = mAlloc.emplace<ir::ExprAccess>();
  access->kind = Ak::STATIC;
  access->idx = nullptr;  // TODO
  access->lval = lowerExpr(exprStAcc->expr);
  access->type = *sema::Type::infer(mAlloc, exprStAcc);
  return access;
}

ir::Expr* via::IRBuilder::lowerExprDynamicAccess(
  const ast::ExprDynAccess* exprDynAcc)
{
  auto* access = mAlloc.emplace<ir::ExprAccess>();
  access->kind = Ak::DYNAMIC;
  access->idx = nullptr;  // TODO
  access->lval = lowerExpr(exprDynAcc->expr);
  return access;
}

ir::Expr* via::IRBuilder::lowerExprUnary(const ast::ExprUnary* exprUnary)
{
  auto* unary = mAlloc.emplace<ir::ExprUnary>();
  unary->op = toUnaryOp(exprUnary->op->kind);
  unary->type = *sema::Type::infer(mAlloc, exprUnary->expr);

  switch (unary->op) {
    case UnaryOp::REF:
      if (ast::isLValue(exprUnary->expr)) {
        unary->expr = lowerExpr(exprUnary->expr);
      } else {
        mDiags.report<Dk::Error>(
          exprUnary->expr->loc,
          "Invalid unary operation '&' (REF) on a non-lvalue expression");
      }
      break;
    case UnaryOp::NEG:
      if (unary->type->isArithmetic()) {
        unary->expr = lowerExpr(exprUnary->expr);
      } else {
        mDiags.report<Dk::Error>(
          exprUnary->expr->loc,
          fmt::format("Invalid unary expression '-' (NEG) on non-arithmetic "
                      "expression of type '{}'",
                      unary->type->dump()));
      }
      break;
    case UnaryOp::NOT:
      unary->type = mTypeCtx.getBuiltinTypeInstance(Btk::Bool);
      unary->expr = lowerExpr(exprUnary->expr);
      break;
    case UnaryOp::BNOT:
      if (unary->type->isIntegral()) {
        unary->expr = lowerExpr(exprUnary->expr);
      } else {
        mDiags.report<Dk::Error>(
          exprUnary->expr->loc,
          fmt::format("Invalid unary expression '~' (BNOT) on non-integral "
                      "expression of type '{}'",
                      unary->type->dump()));
      }
      break;
  }

  return unary;
}

ir::Expr* via::IRBuilder::lowerExprBinary(const ast::ExprBinary* exprBinary)
{
  if (auto type = sema::Type::infer(mAlloc, exprBinary)) {
    auto* binary = mAlloc.emplace<ir::ExprBinary>();
    binary->op = toBinaryOp(exprBinary->op->kind);
    binary->lhs = lowerExpr(exprBinary->lhs);
    binary->rhs = lowerExpr(exprBinary->rhs);
    binary->type = *type;
    return binary;
  } else {
    mDiags.report<Dk::Error>(exprBinary->loc, "Invalid binary expression");
    return nullptr;
  }
}

ir::Expr* via::IRBuilder::lowerExprGroup(const ast::ExprGroup* exprGroup)
{
  return lowerExpr(exprGroup->expr);
}

ir::Expr* via::IRBuilder::lowerExprCall(const ast::ExprCall* exprCall) {}

ir::Expr* via::IRBuilder::lowerExprSubscript(
  const ast::ExprSubscript* exprSubsc)
{}

ir::Expr* via::IRBuilder::lowerExprCast(const ast::ExprCast* exprCast) {}

ir::Expr* via::IRBuilder::lowerExprTernary(const ast::ExprTernary* exprTernary)
{}

ir::Expr* via::IRBuilder::lowerExprArray(const ast::ExprArray* exprArray) {}

ir::Expr* via::IRBuilder::lowerExprTuple(const ast::ExprTuple* exprTuple) {}

ir::Expr* via::IRBuilder::lowerExprLambda(const ast::ExprLambda* exprLambda) {}

ir::Expr* via::IRBuilder::lowerExpr(const ast::Expr* expr) {}

via::IRTree via::IRBuilder::build()
{
  sema::stack::reset();

  IRTree tree;
  return tree;
}
