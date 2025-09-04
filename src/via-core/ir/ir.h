// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_IR_H_
#define VIA_CORE_IR_H_

#include <via/config.h>
#include <via/types.h>
#include "module/symbol.h"
#include "sema/const_value.h"
#include "sema/type.h"
#include "visitor.h"

namespace via
{

namespace ir
{

struct Expr
{
  virtual void accept(Visitor& vis, VisitInfo* vi) const = 0;
  virtual String dump(usize& depth) const = 0;
};

struct Stmt
{
  virtual void accept(Visitor& vis, VisitInfo* vi) const = 0;
  virtual String dump(usize& depth) const = 0;
  virtual Optional<SymbolId> getSymbol() const { return nullopt; }
};

struct Terminator
{
  virtual void accept(Visitor& vis, VisitInfo* vi) const = 0;
  virtual String dump(usize& depth) const = 0;
};

#define NODE_FIELDS()                                     \
  String dump(usize& depth) const override;               \
  void accept(Visitor& vis, VisitInfo* vi) const override \
  {                                                       \
    vis.visit(*this, vi);                                 \
  }

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
  const sema::Type* type;

  String dump() const;
};

struct ExprConstant : public Expr
{
  NODE_FIELDS()
  sema::ConstValue cv;
};

struct ExprSymbol : public Expr
{
  NODE_FIELDS()
  SymbolId symbol;
};

struct ExprAccess : public Expr
{
  NODE_FIELDS()

  enum class Kind
  {
    STATIC,
    DYNAMIC,
  } kind;

  Expr *lval, *idx;
};

struct ExprUnary : public Expr
{
  NODE_FIELDS()

  enum class Op
  {
    NEG,
    NOT,
    BNOT,
  } op;

  Expr* expr;
};

struct ExprBinary : public Expr
{
  NODE_FIELDS()

  enum class Op
  {
    ADD,
    SUB,
    MUL,
    DIV,
    POW,
    MOD,
    AND,
    OR,
    BAND,
    BOR,
    BXOR,
    BSHL,
    BSHR,
  } op;

  Expr *lhs, *rhs;
};

struct ExprCall : public Expr
{
  NODE_FIELDS()
  Expr* callee;
  Vec<Expr*> args;
};

struct ExprSubscript : public Expr
{
  NODE_FIELDS()
  Expr *expr, *idx;
};

struct ExprCast : public Expr
{
  NODE_FIELDS()
  Expr* expr;
  const sema::Type* type;
};

struct ExprTuple : public Expr
{
  NODE_FIELDS()
  Vec<Expr*> init;
};

struct Function;
struct ExprLambda : public Expr
{
  NODE_FIELDS()
};

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

  Optional<SymbolId> getSymbol() const override { return sym; }
};

struct StmtBlock : public Stmt
{
  NODE_FIELDS()
  SymbolId name;
  Vec<Stmt*> stmts;
  Terminator* term;
};

}  // namespace ir

using IrTree = Vec<ir::Stmt*>;

namespace debug
{

[[nodiscard]] String dump(const IrTree& ir);

}

}  // namespace via

#endif
