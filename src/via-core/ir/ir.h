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
  virtual void accept(Visitor& vis, VisitInfo* vi) = 0;
  virtual String dump() const = 0;
};

struct Stmt
{
  virtual void accept(Visitor& vis, VisitInfo* vi) = 0;
  virtual String dump() const = 0;
};

struct Entity
{
  SymbolId symbol;
  virtual void accept(Visitor& vis, VisitInfo* vi) = 0;
  virtual String dump() const = 0;
};

struct Terminator
{
  virtual void accept(Visitor& vis, VisitInfo* vi) = 0;
  virtual String dump() const = 0;
};

#define NODE_FIELDS()                               \
  String dump() const override                      \
  {                                                 \
    debug::todo("dump");                            \
  }                                                 \
  void accept(Visitor& vis, VisitInfo* vi) override \
  {                                                 \
    vis.visit(*this, vi);                           \
  }

struct Return : public Terminator
{
  NODE_FIELDS()
  const Expr* val;
};

struct Continue : public Terminator
{
  NODE_FIELDS()
};

struct Break : public Terminator
{
  NODE_FIELDS()
};

struct Br : public Terminator
{
  NODE_FIELDS()
  SymbolId lbl;
};

struct CondBr : public Terminator
{
  NODE_FIELDS()
  Expr* cnd;
  SymbolId iftrue, iffalse;
};

struct BasicBlock
{
  SymbolId name;
  Vec<Stmt*> stmts;
  Terminator* term;

  String dump();
};

struct Parameter
{
  SymbolId sym;
  sema::Type* type;

  String dump();
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

  QualPath qs;
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
    OP_STAR_STAR,
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
  sema::Type* type;
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
  Function* fn;
};

struct StmtVarDecl : public Stmt
{
  NODE_FIELDS()
  SymbolId sym;
  Expr* expr;
};

struct Function : public Entity
{
  NODE_FIELDS()

  using Entity::symbol;

  enum class Kind
  {
    IR,
    NATIVE,
  } kind;

  SymbolId sym;
  sema::Type* ret;
  Vec<Parameter> parms;
};

struct Module : public Entity
{
  NODE_FIELDS()

  using Entity::symbol;
};

struct Type : public Entity
{
  NODE_FIELDS()

  using Entity::symbol;
};

struct Enum : public Entity
{
  NODE_FIELDS()

  using Entity::symbol;
};

struct Block : public Entity
{
  NODE_FIELDS()

  using Entity::symbol;
};

}  // namespace ir

using IrTree = Vec<ir::Entity*>;

namespace debug
{

[[nodiscard]] String dump(const IrTree& ir);

}

}  // namespace via

#endif
