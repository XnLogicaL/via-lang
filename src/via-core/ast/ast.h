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
#include "lexer/location.h"
#include "lexer/token.h"

#define TRY_COERCE(T, a, b) (T* a = dynamic_cast<T*>(b))
#define TRY_IS(T, a) (dynamic_cast<T*>(a) != nullptr)

namespace via
{

enum class UnaryOp
{
  REF,   // &
  NEG,   // -
  NOT,   // not
  BNOT,  // ~
};

enum class BinaryOp
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
};

UnaryOp toUnaryOp(Token::Kind kind) noexcept;
BinaryOp toBinaryOp(Token::Kind kind) noexcept;

namespace ast
{

struct Expr
{
  SourceLoc loc;

  virtual std::string dump(usize& depth) const = 0;
};

struct Stmt
{
  SourceLoc loc;

  virtual std::string dump(usize& depth) const = 0;
};

struct Type
{
  SourceLoc loc;

  virtual std::string dump(usize& depth) const = 0;
};

struct AccessIdent
{
  bool inst;
  const Token* symbol;
  Vec<const Type*> gens;
  SourceLoc loc;

  std::string dump() const;
};

struct Path
{
  Vec<const Token*> path;
  SourceLoc loc;

  std::string dump() const;
};

struct Parameter
{
  const Token* sym;
  const Type* type;
  SourceLoc loc;

  std::string dump() const;
};

struct AttributeGroup
{
  struct Attribute
  {
    const Path* sp;
    Vec<const Token*> args;
  };

  Vec<Attribute> ats;
  SourceLoc loc;

  std::string dump() const;
};

#define NODE_FIELDS(base) \
  using base::loc;        \
  std::string dump(usize& depth) const override;

struct ExprLit : public Expr
{
  NODE_FIELDS(Expr)
  const Token* tok;
};

struct ExprSymbol : public Expr
{
  NODE_FIELDS(Expr)
  const Token* sym;
};

struct ExprDynAccess : public Expr
{
  NODE_FIELDS(Expr)
  const Expr* expr;
  const AccessIdent* aid;
};

struct ExprStaticAccess : public Expr
{
  NODE_FIELDS(Expr)
  const Expr* expr;
  const AccessIdent* aid;
  Vec<const Type*> gens;
};

struct ExprUnary : public Expr
{
  NODE_FIELDS(Expr)
  const Token* op;
  const Expr* expr;
};

struct ExprBinary : public Expr
{
  NODE_FIELDS(Expr)
  const Token* op;
  const Expr *lhs, *rhs;
};

struct ExprGroup : public Expr
{
  NODE_FIELDS(Expr)
  const Expr* expr;
};

struct ExprCall : public Expr
{
  NODE_FIELDS(Expr)
  const Expr* lval;
  Vec<const Expr*> args;
};

struct ExprSubscript : public Expr
{
  NODE_FIELDS(Expr)
  const Expr *lval, *idx;
};

struct ExprCast : public Expr
{
  NODE_FIELDS(Expr)
  const Expr* expr;
  const Type* type;
};

struct ExprTernary : public Expr
{
  NODE_FIELDS(Expr)
  const Expr *cnd, *lhs, *rhs;
};

struct ExprArray : public Expr
{
  NODE_FIELDS(Expr)
  Vec<const Expr*> init;
};

struct ExprTuple : public Expr
{
  NODE_FIELDS(Expr)
  Vec<const Expr*> vals;
};

struct StmtScope;

struct ExprLambda : public Expr
{
  NODE_FIELDS(Expr)
  Vec<const Parameter*> pms;
  const StmtScope* scope;
};

struct StmtVarDecl : public Stmt
{
  NODE_FIELDS(Stmt)
  const Token* decl;
  const Expr* lval;
  const Expr* rval;
  const Type* type;
};

struct StmtScope : public Stmt
{
  NODE_FIELDS(Stmt)
  Vec<const Stmt*> stmts;
};

struct StmtIf : public Stmt
{
  struct Branch
  {
    const Expr* cnd;
    const StmtScope* br;
  };

  NODE_FIELDS(Stmt)
  Vec<Branch> brs;
};

struct StmtFor : public Stmt
{
  NODE_FIELDS(Stmt)
  const StmtVarDecl* init;
  const Expr *target, *step;
  const StmtScope* br;
};

struct StmtForEach : public Stmt
{
  NODE_FIELDS(Stmt)
  const Expr* lval;
  const Expr* iter;
  const StmtScope* br;
};

struct StmtWhile : public Stmt
{
  NODE_FIELDS(Stmt)
  const Expr* cnd;
  const StmtScope* br;
};

struct StmtAssign : public Stmt
{
  NODE_FIELDS(Stmt)
  const Token* op;
  const Expr *lval, *rval;
};

struct StmtReturn : public Stmt
{
  NODE_FIELDS(Stmt)
  const Expr* expr;
};

struct StmtEnum : public Stmt
{
  struct Pair
  {
    const Token* sym;
    const Expr* expr;
  };

  NODE_FIELDS(Stmt);
  const Token* sym;
  const Type* type;
  Vec<Pair> pairs;
};

struct StmtModule : public Stmt
{
  NODE_FIELDS(Stmt);
  const Token* sym;
  Vec<const Stmt*> scp;
};

struct StmtImport : public Stmt
{
  NODE_FIELDS(Stmt);

  enum class TailKind
  {
    Import,          // a::b
    ImportAll,       // a::*
    ImportCompound,  // a::{b...}
  } kind;

  Vec<const Token*> path;
  Vec<const Token*> tail;
};

struct StmtFunctionDecl : public Stmt
{
  NODE_FIELDS(Stmt);

  const Token* name;
  const Type* ret;
  Vec<const Parameter*> parms;
  const StmtScope* scp;
};

struct StmtStructDecl : public Stmt
{
  NODE_FIELDS(Stmt);

  const Token* name;
  Vec<const Stmt*> scp;
};

struct StmtTypeDecl : public Stmt
{
  NODE_FIELDS(Stmt);
  const Token* sym;
  const Type* type;
};

struct StmtUsing : public Stmt
{
  NODE_FIELDS(Stmt);
  const Path* sp;
  const StmtScope* scp;
};

struct StmtEmpty : public Stmt
{
  NODE_FIELDS(Stmt)
};

struct StmtExpr : public Stmt
{
  NODE_FIELDS(Stmt)
  const Expr* expr;
};

struct TypeBuiltin : public Type
{
  NODE_FIELDS(Type);
  const Token* tok;
};

struct TypeArray : public Type
{
  NODE_FIELDS(Type);
  const Type* type;
};

struct TypeDict : public Type
{
  NODE_FIELDS(Type);
  const Type *key, *val;
};

struct TypeFunc : public Type
{
  NODE_FIELDS(Type);
  const Type* ret;
  Vec<const Parameter*> params;
};

#undef NODE_FIELDS

bool isLValue(const Expr* expr) noexcept;

}  // namespace ast

using SyntaxTree = Vec<const ast::Stmt*>;

namespace debug
{

[[nodiscard]] std::string dump(const SyntaxTree& ast);

}

}  // namespace via
