// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_IR_IR_H_
#define VIA_CORE_IR_IR_H_

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

struct Parameter
{
  SymbolId symbol;
  const sema::Type* type;
};

struct Expr
{
  virtual void accept(Visitor& vis, VisitInfo* vi) = 0;
  virtual String to_string() = 0;
};

struct Stmt
{
  virtual void accept(Visitor& vis, VisitInfo* vi) = 0;
  virtual String to_string() = 0;
};

struct Entity
{
  SymbolId symbol;
  virtual void accept(Visitor& vis, VisitInfo* vi) = 0;
  virtual String to_string() = 0;
};

struct Terminator
{
  virtual void accept(Visitor& vis, VisitInfo* vi) = 0;
  virtual String to_string() = 0;
};

#define COMMON_HEADER()                             \
  void accept(Visitor& vis, VisitInfo* vi) override \
  {                                                 \
    vis.visit(*this, vi);                           \
  }                                                 \
  String to_string() override                       \
  {                                                 \
    debug::unimplemented();                         \
  }

struct Return : public Terminator
{
  COMMON_HEADER()
  const Expr* val;
};

struct Continue : public Terminator
{
  COMMON_HEADER()
};

struct Break : public Terminator
{
  COMMON_HEADER()
};

struct Br : public Terminator
{
  COMMON_HEADER()
  SymbolId lbl;
};

struct CondBr : public Terminator
{
  COMMON_HEADER()
  Expr* cnd;
  SymbolId iftrue, iffalse;
};

struct BasicBlock
{
  SymbolId name;
  Vec<Stmt*> stmts;
  Terminator* term;

  String to_string();
};

struct ExprConstant : public Expr
{
  COMMON_HEADER()
  sema::ConstValue cv;
};

struct ExprSymbol : public Expr
{
  COMMON_HEADER()
  SymbolId symbol;
};

struct ExprAccess : public Expr
{
  COMMON_HEADER()

  enum class Kind
  {
    STATIC,
    DYNAMIC,
  } kind;

  QualPath qs;
};

struct ExprUnary : public Expr
{
  COMMON_HEADER()

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
  COMMON_HEADER()

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
  COMMON_HEADER()
  Expr* callee;
  Vec<Expr*> args;
};

struct ExprSubscript : public Expr
{
  COMMON_HEADER()
  Expr *expr, *idx;
};

struct ExprCast : public Expr
{
  COMMON_HEADER()
  Expr* expr;
  sema::Type* type;
};

struct ExprTuple : public Expr
{
  COMMON_HEADER()
  Vec<Expr*> init;
};

struct Function;
struct ExprLambda : public Expr
{
  COMMON_HEADER()
  Function* fn;
};

struct StmtVarDecl : public Stmt
{
  COMMON_HEADER()
  SymbolId sym;
  Expr* expr;
};

struct Function : public Entity
{
  using Entity::symbol;
  COMMON_HEADER()
};

struct Module : public Entity
{
  using Entity::symbol;
  COMMON_HEADER()
};

struct Type : public Entity
{
  using Entity::symbol;
  COMMON_HEADER()
};

struct Enum : public Entity
{
  using Entity::symbol;
  COMMON_HEADER()
};

struct Block : public Entity
{
  using Entity::symbol;
  COMMON_HEADER()
};

using IrTree = Vec<Entity*>;

}  // namespace ir

}  // namespace via

#endif
