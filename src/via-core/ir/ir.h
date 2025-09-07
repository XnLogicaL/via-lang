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
#include "module/symbol.h"
#include "option.h"
#include "sema/const_value.h"
#include "sema/local.h"
#include "sema/type.h"

namespace via
{

namespace ir
{

struct Expr
{
  const sema::Type* type;
  virtual std::string dump(usize& depth) const = 0;
};

struct Stmt
{
  virtual std::string dump(usize& depth) const = 0;
  virtual Option<SymbolId> getSymbol() const { return nullopt; }
};

struct Terminator
{
  virtual std::string dump(usize& depth) const = 0;
};

#define NODE_FIELDS() std::string dump(usize& depth) const override;

struct TrReturn : public Terminator
{
  NODE_FIELDS()
  const Expr* val;
};

struct TrContinue : public Terminator
{
  NODE_FIELDS()
};

struct TrBreak : public Terminator
{
  NODE_FIELDS()
};

struct TrBranch : public Terminator
{
  NODE_FIELDS()
  usize lbl;
};

struct TrCondBranch : public Terminator
{
  NODE_FIELDS()
  Expr* cnd;
  usize iftrue, iffalse;
};

struct Parm
{
  SymbolId sym;
  sema::Type* type;

  std::string dump() const;
};

#undef NODE_FIELDS
#define NODE_FIELDS(base) \
  using base::type;       \
  std::string dump(usize& depth) const override;

struct ExprConstant : public Expr
{
  NODE_FIELDS(Expr)
  sema::ConstValue value;
};

struct ExprSymbol : public Expr
{
  NODE_FIELDS(Expr)
  SymbolId symbol;
  sema::LocalRef* local;
};

struct ExprAccess : public Expr
{
  NODE_FIELDS(Expr)

  enum class Kind
  {
    STATIC,
    DYNAMIC,
  } kind;

  Expr *lval, *idx;
};

struct ExprUnary : public Expr
{
  NODE_FIELDS(Expr)
  UnaryOp op;
  Expr* expr;
};

struct ExprBinary : public Expr
{
  NODE_FIELDS(Expr)
  BinaryOp op;
  Expr *lhs, *rhs;
};

struct ExprCall : public Expr
{
  NODE_FIELDS(Expr)
  Expr* callee;
  Vec<Expr*> args;
};

struct ExprSubscript : public Expr
{
  NODE_FIELDS(Expr)
  Expr *expr, *idx;
};

struct ExprCast : public Expr
{
  NODE_FIELDS(Expr)
  Expr* expr;
  const sema::Type* type;
};

struct ExprTernary : public Expr
{
  NODE_FIELDS(Expr)
  Expr *cnd, *iftrue, *iffalse;
};

struct ExprArray : public Expr
{
  NODE_FIELDS(Expr)
  Vec<Expr*> exprs;
};

struct ExprTuple : public Expr
{
  NODE_FIELDS(Expr)
  Vec<Expr*> init;
};

struct Function;
struct ExprLambda : public Expr
{
  NODE_FIELDS(Expr)
};

#undef NODE_FIELDS
#define NODE_FIELDS() std::string dump(usize& depth) const override;

struct StmtVarDecl : public Stmt
{
  NODE_FIELDS()
  SymbolId sym;
  Expr* expr;
};

struct StmtBlock;

struct StmtFuncDecl : public Stmt
{
  NODE_FIELDS()

  enum class Kind
  {
    IR,
    NATIVE,
  } kind;

  SymbolId sym;
  const sema::Type* ret;
  Vec<Parm> parms;
  StmtBlock* body;

  Option<SymbolId> getSymbol() const override { return sym; }
};

struct StmtBlock : public Stmt
{
  NODE_FIELDS()
  SymbolId name;
  Vec<Stmt*> stmts;
  Terminator* term;
};

}  // namespace ir

using IRTree = Vec<ir::Stmt*>;

namespace debug
{

[[nodiscard]] std::string dump(const IRTree& ir);

}

}  // namespace via
