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
#include "sema/type.h"

namespace via
{

class Module;
struct Def;

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

struct Term
{
  virtual std::string dump(usize& depth) const = 0;
};

#define NODE_FIELDS() std::string dump(usize& depth) const override;

struct TrReturn : public Term
{
  NODE_FIELDS()
  const Expr* val;
};

struct TrContinue : public Term
{
  NODE_FIELDS()
};

struct TrBreak : public Term
{
  NODE_FIELDS()
};

struct TrBranch : public Term
{
  NODE_FIELDS()
  usize lbl;
};

struct TrCondBranch : public Term
{
  NODE_FIELDS()
  const Expr* cnd;
  usize iftrue, iffalse;
};

struct Parm
{
  SymbolId sym;
  const sema::Type* type;
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
};

struct ExprAccess : public Expr
{
  NODE_FIELDS(Expr)

  enum class Kind
  {
    STATIC,
    DYNAMIC,
  } kind;

  const Expr* root;
  SymbolId index;
};

struct ExprModuleAccess : public Expr
{
  NODE_FIELDS(Expr)
  Module* module;
  SymbolId index;
  const Def* def;
};

struct ExprUnary : public Expr
{
  NODE_FIELDS(Expr)
  UnaryOp op;
  const Expr* expr;
};

struct ExprBinary : public Expr
{
  NODE_FIELDS(Expr)
  BinaryOp op;
  const Expr *lhs, *rhs;
};

struct ExprCall : public Expr
{
  NODE_FIELDS(Expr)
  const Expr* callee;
  Vec<const Expr*> args;
};

struct ExprSubscript : public Expr
{
  NODE_FIELDS(Expr)
  const Expr *expr, *idx;
};

struct ExprCast : public Expr
{
  NODE_FIELDS(Expr)
  const Expr* expr;
  const sema::Type* cast;
};

struct ExprTernary : public Expr
{
  NODE_FIELDS(Expr)
  const Expr *cnd, *iftrue, *iffalse;
};

struct ExprArray : public Expr
{
  NODE_FIELDS(Expr)
  Vec<const Expr*> exprs;
};

struct ExprTuple : public Expr
{
  NODE_FIELDS(Expr)
  Vec<const Expr*> init;
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
  const Expr* expr;
  const sema::Type* declType;
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
  const StmtBlock* body;

  Option<SymbolId> getSymbol() const override { return sym; }
};

struct StmtBlock : public Stmt
{
  NODE_FIELDS()
  SymbolId name;
  Vec<const Stmt*> stmts;
  const Term* term;
};

struct StmtExpr : public Stmt
{
  NODE_FIELDS()
  const Expr* expr;
};

}  // namespace ir

using IRTree = Vec<const ir::Stmt*>;

namespace debug
{

[[nodiscard]] std::string dump(const IRTree& ir);

}

}  // namespace via
