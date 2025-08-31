// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_AST_H_
#define VIA_CORE_AST_H_

#include <via/config.h>
#include <via/types.h>
#include "debug.h"
#include "lexer/location.h"
#include "lexer/token.h"
#include "visitor.h"

#define TRY_COERCE(T, a, b) (T* a = dynamic_cast<T*>(b))

namespace via
{

namespace ast
{

struct Expr
{
  SourceLoc loc;

  virtual String dump(usize& depth) const = 0;
  virtual void accept(Visitor& vis, VisitInfo* vi) const = 0;
};

struct Stmt
{
  SourceLoc loc;

  virtual String dump(usize& depth) const = 0;
  virtual void accept(Visitor& vis, VisitInfo* vi) const = 0;
};

struct Type
{
  SourceLoc loc;

  virtual String dump(usize& depth) const = 0;
  virtual void accept(Visitor& vis, VisitInfo* vi) const = 0;
};

struct TupleBinding
{
  Vec<const Token*> binds;
  SourceLoc loc;

  String dump() const;
};

struct Path
{
  Vec<const Token*> path;
  SourceLoc loc;

  String dump() const;
};

struct LValue
{
  enum class Kind
  {
    SYM,  // Symbol
    TPB,  // Tuple binding
    SP,   // Static path
    DP,   // Dynamic path
  } kind;

  union
  {
    const Token* sym;
    const Path* path;
    const TupleBinding* tpb;
  };

  SourceLoc loc;

  String dump() const;
};

struct PlValue
{
  enum class Kind
  {
    SYM,  // Symbol
    SP,   // Static path
    DP,   // Dynamic path
  } kind;

  union
  {
    const Token* sym;
    const Path* path;
  };

  SourceLoc loc;

  String dump() const;
};

struct Parameter
{
  const Token* sym;
  const Type* type;
  SourceLoc loc;

  String dump() const;
};

struct AttributeGroup
{
  struct Attribute
  {
    const Path* mStkPtr;
    Vec<const Token*> args;
  };

  Vec<Attribute> ats;
  SourceLoc loc;

  String dump() const;
};

#define COMMON_HEADER(klass)                              \
  using klass::loc;                                       \
  String dump(usize& depth) const override;               \
  void accept(Visitor& vis, VisitInfo* vi) const override \
  {                                                       \
    vis.visit(*this, vi);                                 \
  }

struct ExprLit : public Expr
{
  COMMON_HEADER(Expr)
  const Token* tok;
};

struct ExprSymbol : public Expr
{
  COMMON_HEADER(Expr)
  const Token* sym;
};

struct ExprDynAccess : public Expr
{
  COMMON_HEADER(Expr)
  const Expr* expr;
  const Token* index;
};

struct ExprStaticAccess : public Expr
{
  COMMON_HEADER(Expr)
  const Expr* expr;
  const Token* index;
};

struct ExprUnary : public Expr
{
  COMMON_HEADER(Expr)
  const Token* op;
  const Expr* expr;
};

struct ExprBinary : public Expr
{
  COMMON_HEADER(Expr)
  const Token* op;
  const Expr *lhs, *rhs;
};

struct ExprGroup : public Expr
{
  COMMON_HEADER(Expr)
  const Expr* expr;
};

struct ExprCall : public Expr
{
  COMMON_HEADER(Expr)
  const Expr* lval;
  Vec<const Expr*> args;
};

struct ExprSubscript : public Expr
{
  COMMON_HEADER(Expr)
  const Expr *lval, *idx;
};

struct ExprCast : public Expr
{
  COMMON_HEADER(Expr)
  const Expr* expr;
  const Type* type;
};

struct ExprTernary : public Expr
{
  COMMON_HEADER(Expr)
  const Expr *cnd, *lhs, *rhs;
};

struct ExprArray : public Expr
{
  COMMON_HEADER(Expr)
  Vec<const Expr*> init;
};

struct ExprTuple : public Expr
{
  COMMON_HEADER(Expr)
  Vec<const Expr*> vals;
};

struct StmtScope;

struct ExprLambda : public Expr
{
  COMMON_HEADER(Expr)
  Vec<const Parameter*> pms;
  const StmtScope* scope;
};

struct StmtVarDecl : public Stmt
{
  COMMON_HEADER(Stmt)
  const Token* decl;
  const LValue* lval;
  const Expr* rval;
  const Type* type;
};

struct StmtScope : public Stmt
{
  COMMON_HEADER(Stmt)
  Vec<const Stmt*> stmts;
};

struct StmtIf : public Stmt
{
  struct Branch
  {
    const Expr* cnd;
    const StmtScope* br;
  };

  COMMON_HEADER(Stmt)
  Vec<Branch> brs;
};

struct StmtFor : public Stmt
{
  COMMON_HEADER(Stmt)
  const StmtVarDecl* init;
  const Expr *target, *step;
  const StmtScope* br;
};

struct StmtForEach : public Stmt
{
  COMMON_HEADER(Stmt)
  const LValue* lval;
  const Expr* iter;
  const StmtScope* br;
};

struct StmtWhile : public Stmt
{
  COMMON_HEADER(Stmt)
  const Expr* cnd;
  const StmtScope* br;
};

struct StmtAssign : public Stmt
{
  COMMON_HEADER(Stmt)
  const Token* op;
  const Expr *lval, *rval;
};

struct StmtReturn : public Stmt
{
  COMMON_HEADER(Stmt)
  const Expr* expr;
};

struct StmtEnum : public Stmt
{
  struct Pair
  {
    const Token* sym;
    const Expr* expr;
  };

  COMMON_HEADER(Stmt);
  const Token* sym;
  const Type* type;
  Vec<Pair> pairs;
};

struct StmtModule : public Stmt
{
  COMMON_HEADER(Stmt);
  const Token* sym;
  Vec<const Stmt*> scp;
};

struct StmtImport : public Stmt
{
  COMMON_HEADER(Stmt);

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
  COMMON_HEADER(Stmt);

  const Token* name;
  const Type* ret;
  Vec<const Parameter*> parms;
  const StmtScope* scp;
};

struct StmtStructDecl : public Stmt
{
  COMMON_HEADER(Stmt);

  const Token* name;
  Vec<const Stmt*> scp;
};

struct StmtTypeDecl : public Stmt
{
  COMMON_HEADER(Stmt);
  const Token* sym;
  const Type* type;
};

struct StmtUsing : public Stmt
{
  COMMON_HEADER(Stmt);
  const Path* mStkPtr;
  const StmtScope* scp;
};

struct StmtEmpty : public Stmt
{
  COMMON_HEADER(Stmt)
};

struct StmtExpr : public Stmt
{
  COMMON_HEADER(Stmt)
  const Expr* expr;
};

struct TypeBuiltin : public Type
{
  COMMON_HEADER(Type);
  const Token* tok;
};

struct TypeArray : public Type
{
  COMMON_HEADER(Type);
  const Type* type;
};

struct TypeDict : public Type
{
  COMMON_HEADER(Type);
  const Type *key, *val;
};

struct TypeFunc : public Type
{
  COMMON_HEADER(Type);
  const Type* ret;
  Vec<const Parameter*> params;
};

#undef COMMON_HEADER

}  // namespace ast

using SyntaxTree = Vec<const ast::Stmt*>;

namespace debug
{

[[nodiscard]] String dump(const SyntaxTree& ast);

}

}  // namespace via

#endif
