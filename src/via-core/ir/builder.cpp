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
  auto* binary = mAlloc.emplace<ir::ExprBinary>();
  binary->op = toBinaryOp(exprBinary->op->kind);
  binary->lhs = lowerExpr(exprBinary->lhs);
  binary->rhs = lowerExpr(exprBinary->rhs);

  switch (binary->op) {
    case BinaryOp::ADD:
    case BinaryOp::SUB:
    case BinaryOp::MUL:
    case BinaryOp::DIV:
    case BinaryOp::POW:
    case BinaryOp::MOD:
    case BinaryOp::AND:
    case BinaryOp::OR:
    case BinaryOp::BAND:
    case BinaryOp::BOR:
    case BinaryOp::BXOR:
    case BinaryOp::BSHL:
    case BinaryOp::BSHR:
    default:
      break;
  }

  return binary;
}

ir::Expr* via::IRBuilder::lowerExprGroup(const ast::ExprGroup* exprGroup)
{
  return lowerExpr(exprGroup->expr);
}

ir::Expr* via::IRBuilder::lowerExprCall(const ast::ExprCall* exprCall)
{
  return nullptr;
}

ir::Expr* via::IRBuilder::lowerExprSubscript(
  const ast::ExprSubscript* exprSubsc)
{
  return nullptr;
}

ir::Expr* via::IRBuilder::lowerExprCast(const ast::ExprCast* exprCast)
{
  return nullptr;
}

ir::Expr* via::IRBuilder::lowerExprTernary(const ast::ExprTernary* exprTernary)
{
  return nullptr;
}

ir::Expr* via::IRBuilder::lowerExprArray(const ast::ExprArray* exprArray)
{
  return nullptr;
}

ir::Expr* via::IRBuilder::lowerExprTuple(const ast::ExprTuple* exprTuple)
{
  return nullptr;
}

ir::Expr* via::IRBuilder::lowerExprLambda(const ast::ExprLambda* exprLambda)
{
  return nullptr;
}

ir::Expr* via::IRBuilder::lowerExpr(const ast::Expr* expr)
{
#define CASE(type, handler)               \
  if TRY_COERCE (const type, inner, expr) \
    return handler(inner);

  CASE(ast::ExprLit, lowerExprLit)
  CASE(ast::ExprSymbol, lowerExprSymbol)
  CASE(ast::ExprStaticAccess, lowerExprStaticAccess)
  CASE(ast::ExprDynAccess, lowerExprDynamicAccess)
  CASE(ast::ExprUnary, lowerExprUnary)
  CASE(ast::ExprBinary, lowerExprBinary)
  CASE(ast::ExprGroup, lowerExprGroup)
  CASE(ast::ExprCall, lowerExprCall)
  CASE(ast::ExprSubscript, lowerExprSubscript)
  CASE(ast::ExprCast, lowerExprCast)
  CASE(ast::ExprTernary, lowerExprTernary)
  CASE(ast::ExprArray, lowerExprArray)
  CASE(ast::ExprTuple, lowerExprTuple)
  CASE(ast::ExprLambda, lowerExprLambda)

  debug::bug(
    fmt::format("unhandled case IRBuilder::lowerExpr({})", TYPENAME(*expr)));

#undef CASE
}

via::IRTree via::IRBuilder::build()
{
  sema::stack::reset();

  IRTree tree;
  return tree;
}
