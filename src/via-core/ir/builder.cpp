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
#include "sema/control_path.h"
#include "sema/stack.h"
#include "utility.h"

namespace ir = via::ir;
namespace sema = via::sema;

using Ak = ir::ExprAccess::Kind;
using Btk = sema::BuiltinType::Kind;
using LocalQual = sema::IRLocal::Qual;

#define PMR_CASE(NODE, TYPE, HANDLER)     \
  if TRY_COERCE (const TYPE, inner, NODE) \
    return HANDLER(inner);

#define UNARY_OP_CASE(VALID, RESULT)                                    \
  {                                                                     \
    .validTypes = [](const sema::Type* type) -> bool { return VALID; }, \
    .resultType = [](sema::TypeContext* ctx,                            \
                     const sema::Type* type) -> const sema::Type* {     \
      return RESULT;                                                    \
    },                                                                  \
  }

struct UnaryOpInfo
{
  std::function<bool(const sema::Type*)> validTypes;  // predicate
  std::function<const sema::Type*(sema::TypeContext*, const sema::Type*)>
    resultType;  // compute result
};

static const UnaryOpInfo sUnaryOpTable[] = {
  /* UnaryOp::NEG */ UNARY_OP_CASE(type->isArithmetic(), type),
  /* UnaryOp::NOT */ UNARY_OP_CASE(true, ctx->getBuiltin(Btk::BOOL)),
  /* UnaryOp::BNOT */ UNARY_OP_CASE(type->isIntegral(), type),
};

#define BINARY_OP_CASE(VALID, RESULT)                                        \
  {                                                                          \
    .validTypes = [](const sema::Type* lhs, const sema::Type* rhs) -> bool { \
      return VALID;                                                          \
    },                                                                       \
    .resultType = [](sema::TypeContext* ctx, const sema::Type* lhs,          \
                     const sema::Type* rhs) -> const sema::Type* {           \
      return RESULT;                                                         \
    },                                                                       \
  }

struct BinaryOpInfo
{
  std::function<bool(const sema::Type*, const sema::Type*)>
    validTypes;  // predicate
  std::function<
    const sema::Type*(sema::TypeContext*, const sema::Type*, const sema::Type*)>
    resultType;  // compute result
};

#define BINARY_OP_PROMOTE(LHS, RHS) \
  ctx->getBuiltin((LHS->isFloat() || RHS->isFloat()) ? Btk::FLOAT : Btk::INT)

static const BinaryOpInfo sBinaryOpTable[] = {
  /* BinaryOp::ADD */
  BINARY_OP_CASE(lhs->isArithmetic() && rhs->isArithmetic(),
                 BINARY_OP_PROMOTE(lhs, rhs)),
  /* BinaryOp::SUB */
  BINARY_OP_CASE(lhs->isArithmetic() && rhs->isArithmetic(),
                 BINARY_OP_PROMOTE(lhs, rhs)),
  /* BinaryOp::MUL */
  BINARY_OP_CASE(lhs->isArithmetic() && rhs->isArithmetic(),
                 BINARY_OP_PROMOTE(lhs, rhs)),
  /* BinaryOp::DIV */
  BINARY_OP_CASE(lhs->isArithmetic() && rhs->isArithmetic(),
                 ctx->getBuiltin(Btk::FLOAT)),
  /* BinaryOp::POW */
  BINARY_OP_CASE(lhs->isArithmetic() && rhs->isArithmetic(),
                 BINARY_OP_PROMOTE(lhs, rhs)),
  /* BinaryOp::MOD */
  BINARY_OP_CASE(lhs->isIntegral() && rhs->isIntegral(),
                 ctx->getBuiltin(Btk::INT)),
  /* BinaryOp::AND */ BINARY_OP_CASE(true, ctx->getBuiltin(Btk::BOOL)),
  /* BinaryOp::OR */ BINARY_OP_CASE(true, ctx->getBuiltin(Btk::BOOL)),
  /* BinaryOp::BAND */
  BINARY_OP_CASE(lhs->isIntegral() && rhs->isIntegral(),
                 ctx->getBuiltin(Btk::INT)),
  /* BinaryOp::BOR */
  BINARY_OP_CASE(lhs->isIntegral() && rhs->isIntegral(),
                 ctx->getBuiltin(Btk::INT)),
  /* BinaryOp::BXOR */
  BINARY_OP_CASE(lhs->isIntegral() && rhs->isIntegral(),
                 ctx->getBuiltin(Btk::INT)),
  /* BinaryOp::BSHL */
  BINARY_OP_CASE(lhs->isIntegral() && rhs->isIntegral(),
                 ctx->getBuiltin(Btk::INT)),
  /* BinaryOp::BSHR */
  BINARY_OP_CASE(lhs->isIntegral() && rhs->isIntegral(),
                 ctx->getBuiltin(Btk::INT)),
};

const sema::Type* via::IRBuilder::typeOf(const ast::Expr* expr) noexcept
{
  using enum Token::Kind;
  using enum sema::BuiltinType::Kind;

  if TRY_COERCE (const ast::ExprLit, lit, expr) {
    sema::BuiltinType::Kind kind;

    switch (lit->tok->kind) {
      case LIT_NIL:
        kind = NIL;
        break;
      case LIT_TRUE:
      case LIT_FALSE:
        kind = BOOL;
        break;
      case LIT_INT:
      case LIT_XINT:
      case LIT_BINT:
        kind = INT;
        break;
      case LIT_FLOAT:
        kind = FLOAT;
        break;
      case LIT_STRING:
        kind = STRING;
        break;
      default:
        debug::bug("invalid literal expression");
    }

    return mTypeCtx.getBuiltin(kind);
  } else if TRY_COERCE (const ast::ExprSymbol, symbol, expr) {
    auto& frame = mStack.top();
    std::string symbolStr = symbol->symbol->toString();

    // Local variable
    if (auto local = frame.getLocal(internSymbol(symbol->symbol->toString()))) {
      auto* irDecl = local->local.getIrDecl();
      if TRY_COERCE (const ir::StmtVarDecl, varDecl, irDecl) {
        return varDecl->declType;
      } else if TRY_COERCE (const ir::StmtFuncDecl, funcDecl, irDecl) {
        std::vector<const sema::Type*> parms;
        for (const auto& parm : funcDecl->parms)
          parms.push_back(parm.type);
        return mTypeCtx.getFunction(funcDecl->ret, parms);
      }
    }

    mDiags.report<Level::ERROR>(
      symbol->loc, std::format("Use of undefined symbol '{}'", symbolStr),
      Footnote(Footnote::Kind::HINT,
               std::format("did you mistype '{}' or forget to declare it?",
                           symbolStr)));
    return nullptr;
  } else if TRY_COERCE (const ast::ExprDynAccess, dyna, expr) {
    return nullptr;
  } else if TRY_COERCE (const ast::ExprStaticAccess, sta, expr) {
    return nullptr;
  } else if TRY_COERCE (const ast::ExprUnary, unary, expr) {
    auto* inner = typeOf(unary->expr);

    UnaryOp op = toUnaryOp(unary->op->kind);
    UnaryOpInfo info = sUnaryOpTable[static_cast<u8>(op)];

    if (!info.validTypes(inner)) {
      mDiags.report<Level::ERROR>(
        unary->loc, std::format("Invalid unary operation '{}' ({}) on "
                                "incompatible type '{}'",
                                unary->op->toString(),
                                magic_enum::enum_name(op), inner->toString()));
      return nullptr;
    }

    return info.resultType(&mTypeCtx, inner);
  } else if TRY_COERCE (const ast::ExprBinary, binary, expr) {
    auto* lhs = typeOf(binary->lhs);
    auto* rhs = typeOf(binary->rhs);

    BinaryOp op = toBinaryOp(binary->op->kind);
    BinaryOpInfo info = sBinaryOpTable[static_cast<u8>(op)];

    if (!info.validTypes(lhs, rhs)) {
      mDiags.report<Level::ERROR>(
        binary->loc,
        std::format("Invalid binary operation '{}' ({}) on "
                    "incompatible types '{}' (LEFT) "
                    "'{}' (RIGHT)",
                    binary->op->toString(), magic_enum::enum_name(op),
                    lhs->toString(), rhs->toString()));
      return nullptr;
    }

    return info.resultType(&mTypeCtx, lhs, rhs);
  } else if TRY_COERCE (const ast::ExprTernary, ternary, expr) {
    const sema::Type *lhs = typeOf(ternary->lhs), *rhs = typeOf(ternary->rhs);
    if (lhs == rhs) {
      return lhs;
    } else {
      mDiags.report<Level::ERROR>(
        ternary->loc,
        std::format("Results of ternary expression '{}' and '{}' do not match",
                    lhs->toString(), rhs->toString()));
    }
    return nullptr;
  }

  debug::unimplemented(std::format("typeOf({})", TYPENAME(*expr)));
}

const sema::Type* via::IRBuilder::typeOf(const ast::Type* type) noexcept
{
  using enum Token::Kind;
  using enum sema::BuiltinType::Kind;

  if TRY_COERCE (const ast::TypeBuiltin, typeBuiltin, type) {
    sema::BuiltinType::Kind kind;

    switch (typeBuiltin->tok->kind) {
      case LIT_NIL:
        kind = NIL;
        break;
      case KW_BOOL:
        kind = BOOL;
        break;
      case KW_INT:
        kind = INT;
        break;
      case KW_FLOAT:
        kind = FLOAT;
        break;
      case KW_STRING:
        kind = STRING;
        break;
      default:
        debug::bug("unmapped builtin type token");
    }

    return mTypeCtx.getBuiltin(kind);
  }

  debug::unimplemented(std::format("typeOf({})", TYPENAME(*type)));
}

const ir::Expr* via::IRBuilder::lowerExprLit(const ast::ExprLit* exprLit)
{
  auto* constant = mAlloc.emplace<ir::ExprConstant>();
  constant->loc = exprLit->loc;
  constant->value = *sema::ConstValue::fromToken(*exprLit->tok);
  constant->type = typeOf(exprLit);
  return constant;
}

const ir::Expr* via::IRBuilder::lowerExprSymbol(const ast::ExprSymbol* exprSym)
{
  auto& frame = mStack.top();
  std::string symbolStr = exprSym->symbol->toString();

  auto* symbol = mAlloc.emplace<ir::ExprSymbol>();
  symbol->loc = exprSym->loc;
  symbol->symbol = mSymbolTable.intern(symbolStr);
  symbol->type = typeOf(exprSym);
  return symbol;
}

const ir::Expr* via::IRBuilder::lowerExprStaticAccess(
  const ast::ExprStaticAccess* exprStAcc)
{
  // Check for module thing
  if TRY_COERCE (const ast::ExprSymbol, rootSymbol, exprStAcc->root) {
    ModuleManager* manager = mModule->getManager();

    if (auto* module =
          manager->getModuleByName(rootSymbol->symbol->toString())) {
      SymbolId low = internSymbol(exprStAcc->index->toString());
      if (auto def = module->lookup(low)) {
        auto* maccess = mAlloc.emplace<ir::ExprModuleAccess>();
        maccess->module = module;
        maccess->index = low;
        maccess->def = *def;
        return maccess;
      }
    }
  }

  auto* access = mAlloc.emplace<ir::ExprAccess>();
  access->kind = Ak::STATIC;
  access->root = lowerExpr(exprStAcc->root);
  access->index = internSymbol(*exprStAcc->index);
  access->type = typeOf(exprStAcc);
  access->loc = exprStAcc->loc;
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
  access->loc = exprDynAcc->loc;
  return access;
}

const ir::Expr* via::IRBuilder::lowerExprUnary(const ast::ExprUnary* exprUnary)
{
  auto* unary = mAlloc.emplace<ir::ExprUnary>();
  unary->op = toUnaryOp(exprUnary->op->kind);
  unary->type = typeOf(exprUnary);
  unary->expr = lowerExpr(exprUnary->expr);
  unary->loc = exprUnary->loc;
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
  binary->loc = exprBinary->loc;
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
  call->loc = exprCall->loc;
  call->type = nullptr; /* TODO: Type of this expression should be the return
                           type of the callee function. */
  call->args = [&]() {
    std::vector<const ir::Expr*> args;
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

  debug::unimplemented(
    std::format("case IRBuilder::lowerExpr({})", TYPENAME(*expr)));
}

const ir::Stmt* via::IRBuilder::lowerStmtVarDecl(
  const ast::StmtVarDecl* stmtVarDecl)
{
  auto* decl = mAlloc.emplace<ir::StmtVarDecl>();
  decl->expr = lowerExpr(stmtVarDecl->rval);
  decl->loc = stmtVarDecl->loc;

  if TRY_COERCE (const ast::ExprSymbol, lval, stmtVarDecl->lval) {
    auto* rvalType = typeOf(stmtVarDecl->rval);

    if (stmtVarDecl->type != nullptr) {
      decl->declType = typeOf(stmtVarDecl->type);

      if (decl->declType != rvalType) {
        mDiags.report<Level::ERROR>(
          stmtVarDecl->rval->loc,
          std::format(
            "Expression type '{}' does not match declaration type '{}'",
            rvalType->toString(), decl->declType->toString()));
      }
    } else {
      decl->declType = rvalType;
    }

    decl->symbol = internSymbol(lval->symbol->toString());
  } else {
    debug::bug("bad lvalue");
  }

  mStack.top().setLocal(decl->symbol, stmtVarDecl, decl);
  return decl;
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
  auto* term = mAlloc.emplace<ir::TrReturn>();
  term->implicit = false;
  term->loc = stmtReturn->loc;
  term->val = stmtReturn->expr ? lowerExpr(stmtReturn->expr) : nullptr;
  term->type = stmtReturn->expr
                 ? typeOf(stmtReturn->expr)
                 : mTypeCtx.getBuiltin(sema::BuiltinType::Kind::NIL);

  ir::StmtBlock* block = endBlock();
  block->term = term;
  return block;
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
  if (mStack.size() > 1) {
    mDiags.report<Level::ERROR>(
      stmtImport->loc,
      "Import statements are only allowed in root scope of a module");
    return nullptr;
  }

  QualName qualName;
  for (const Token* tok : stmtImport->path) {
    qualName.push_back(tok->toString());
  }

  std::string name = qualName.back();
  if (auto module = mModule->getManager()->getModuleByName(name)) {
    mDiags.report<Level::ERROR>(
      stmtImport->loc,
      std::format("Module '{}' imported more than once", name));

    if (auto* importDecl = module->getImportDecl()) {
      mDiags.report<Level::INFO>(importDecl->loc, "Previously imported here");
    }
  }

  auto result = mModule->resolveImport(qualName, stmtImport);
  if (result.hasError()) {
    mDiags.report<Level::ERROR>(stmtImport->loc, result.getError().toString());
  }

  return nullptr;
}

const ir::Stmt* via::IRBuilder::lowerStmtFunctionDecl(
  const ast::StmtFunctionDecl* stmtFunctionDecl)
{
  auto* fndecl = mAlloc.emplace<ir::StmtFuncDecl>();
  fndecl->kind = ir::StmtFuncDecl::Kind::IR;
  fndecl->symbol = internSymbol(stmtFunctionDecl->name->toString());
  fndecl->ret = stmtFunctionDecl->ret ? typeOf(stmtFunctionDecl->ret) : nullptr;

  // TODO: Get rid of this
  if (fndecl->ret == nullptr) {
    mDiags.report<Level::ERROR>(
      stmtFunctionDecl->loc,
      "Compiler infered return types are not implemented");
    return nullptr;
  }

  for (const auto& parm : stmtFunctionDecl->parms) {
    ir::Parm newParm;
    newParm.symbol = internSymbol(parm->symbol->toString());
    newParm.type = typeOf(parm->type);

    fndecl->parms.push_back(newParm);
  }

  auto* block = mAlloc.emplace<ir::StmtBlock>();
  block->id = iota<usize>();

  mStack.push({});

  for (const auto& stmt : stmtFunctionDecl->scope->stmts) {
    if TRY_COERCE (const ast::StmtReturn, ret, stmt) {
      auto* term = mAlloc.emplace<ir::TrReturn>();
      term->implicit = false;
      term->loc = ret->loc;
      term->val = ret->expr ? lowerExpr(ret->expr) : nullptr;
      term->type = ret->expr
                     ? typeOf(ret->expr)
                     : mTypeCtx.getBuiltin(sema::BuiltinType::Kind::NIL);
      block->term = term;
      break;
    }

    block->stmts.push_back(lowerStmt(stmt));
  }

  mStack.pop();

  if (block->term == nullptr) {
    SourceLoc loc{stmtFunctionDecl->scope->loc.end - 1,
                  stmtFunctionDecl->scope->loc.end};

    auto* nil = mAlloc.emplace<ir::ExprConstant>();
    nil->loc = loc;
    nil->type = mTypeCtx.getBuiltin(sema::BuiltinType::Kind::NIL);
    nil->value = sema::ConstValue();

    auto* term = mAlloc.emplace<ir::TrReturn>();
    term->implicit = true;
    term->loc = loc;
    term->val = nil;
    term->type = mTypeCtx.getBuiltin(sema::BuiltinType::Kind::NIL);
    block->term = term;
  }

  const sema::Type* expectedRetType = fndecl->ret;

  for (const auto& term : sema::analyzeControlPaths(block)) {
    if TRY_COERCE (const ir::TrReturn, ret, term) {
      if (!ret->type) {
        // Already failed, no need to diagnose further
        continue;
      }

      if (!expectedRetType) {
        expectedRetType = ret->type;
      } else if (expectedRetType != ret->type) {
        Footnote implicitReturnNote =
          ret->implicit
            ? Footnote(Footnote::Kind::NOTE, std::format("Implicit return here",
                                                         ret->type->toString()))
            : Footnote();

        if (fndecl->ret) {
          mDiags.report<Level::ERROR>(
            ret->loc,
            std::format("Function return type '{}' does not match type "
                        "'{}' returned by control path",
                        fndecl->ret->toString(), ret->type->toString()),
            implicitReturnNote);
        } else {
          mDiags.report<Level::ERROR>(
            ret->loc,
            "All code paths must return the same type "
            "in function with inferred return type",
            implicitReturnNote);
        }
        break;
      }
    } else {
      mDiags.report<Level::ERROR>(
        term->loc, "All control paths must return from function");
      break;
    }
  }

  if (fndecl->ret && expectedRetType && fndecl->ret != expectedRetType) {
    mDiags.report<Level::ERROR>(
      block->loc,
      std::format("Function return type '{}' does not match inferred "
                  "return type '{}' from all control paths",
                  fndecl->ret->toString(), expectedRetType->toString()));
  }

  auto& frame = mStack.top();
  frame.setLocal(fndecl->symbol, stmtFunctionDecl, fndecl, LocalQual::CONST);

  fndecl->body = block;
  fndecl->loc = stmtFunctionDecl->loc;
  return fndecl;
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
  expr->loc = stmtExpr->loc;
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

  debug::unimplemented(
    std::format("case IRBuilder::lowerStmt({})", TYPENAME(*stmt)));
}

via::IRTree via::IRBuilder::build()
{
  mStack.push({});          // Push root stack frame
  newBlock(iota<usize>());  // Push block

  IRTree tree;

  for (const auto& astStmt : mAst) {
    if (const ir::Stmt* loweredStmt = lowerStmt(astStmt)) {
      mCurrentBlock->stmts.push_back(loweredStmt);
    }

    if (mShouldPushBlock) {
      ir::StmtBlock* block = newBlock(iota<usize>());
      tree.push_back(block);
    }
  }

  // Push last block (it likely will not have a terminator)
  tree.push_back(endBlock());
  return tree;
}
