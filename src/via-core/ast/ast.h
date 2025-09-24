/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <vector>
#include <via/config.h>
#include <via/types.h>
#include "lexer/location.h"
#include "lexer/token.h"

#define TRY_COERCE(T, a, b) (T* a = dynamic_cast<T*>(b))
#define TRY_IS(T, a) (dynamic_cast<T*>(a) != nullptr)

namespace via {
namespace ast {

struct Expr
{
    SourceLoc loc;
    virtual std::string to_string(size_t& depth) const = 0;
};

struct Stmt
{
    SourceLoc loc;
    virtual std::string to_string(size_t& depth) const = 0;
};

struct Type
{
    SourceLoc loc;
    virtual std::string to_string(size_t& depth) const = 0;
};

struct Path
{
    std::vector<const Token*> path;
    SourceLoc loc;

    std::string to_string() const;
};

struct Parameter
{
    const Token* symbol;
    const Type* type;
    SourceLoc loc;

    std::string to_string() const;
};

struct AttributeGroup
{
    struct Attribute
    {
        const Path* sp;
        std::vector<const Token*> args;
    };

    std::vector<Attribute> ats;
    SourceLoc loc;

    std::string to_string() const;
};

#define NODE_FIELDS(base)                                                                \
    using base::loc;                                                                     \
    std::string to_string(size_t& depth) const override;

struct ExprLiteral: public Expr
{
    NODE_FIELDS(Expr)
    const Token* tok;
};

struct ExprSymbol: public Expr
{
    NODE_FIELDS(Expr)
    const Token* symbol;
};

struct ExprDynAccess: public Expr
{
    NODE_FIELDS(Expr)
    const Expr* root;
    const Token* index;
};

struct ExprStaticAccess: public Expr
{
    NODE_FIELDS(Expr)
    const Expr* root;
    const Token* index;
};

struct ExprUnary: public Expr
{
    NODE_FIELDS(Expr)
    const Token* op;
    const Expr* expr;
};

struct ExprBinary: public Expr
{
    NODE_FIELDS(Expr)
    const Token* op;
    const Expr *lhs, *rhs;
};

struct ExprGroup: public Expr
{
    NODE_FIELDS(Expr)
    const Expr* expr;
};

struct ExprCall: public Expr
{
    NODE_FIELDS(Expr)
    const Expr* lval;
    std::vector<const Expr*> args;
};

struct ExprSubscript: public Expr
{
    NODE_FIELDS(Expr)
    const Expr *lval, *idx;
};

struct ExprCast: public Expr
{
    NODE_FIELDS(Expr)
    const Expr* expr;
    const Type* type;
};

struct ExprTernary: public Expr
{
    NODE_FIELDS(Expr)
    const Expr *cnd, *lhs, *rhs;
};

struct ExprArray: public Expr
{
    NODE_FIELDS(Expr)
    std::vector<const Expr*> init;
};

struct ExprTuple: public Expr
{
    NODE_FIELDS(Expr)
    std::vector<const Expr*> vals;
};

struct StmtScope;

struct ExprLambda: public Expr
{
    NODE_FIELDS(Expr)
    std::vector<const Parameter*> pms;
    const StmtScope* scope;
};

struct StmtVarDecl: public Stmt
{
    NODE_FIELDS(Stmt)
    const Token* decl;
    const Expr* lval;
    const Expr* rval;
    const Type* type;
};

struct StmtScope: public Stmt
{
    NODE_FIELDS(Stmt)
    std::vector<const Stmt*> stmts;
};

struct StmtIf: public Stmt
{
    struct Branch
    {
        const Expr* cnd;
        const StmtScope* br;
    };

    NODE_FIELDS(Stmt)
    std::vector<Branch> brs;
};

struct StmtFor: public Stmt
{
    NODE_FIELDS(Stmt)
    const StmtVarDecl* init;
    const Expr *target, *step;
    const StmtScope* br;
};

struct StmtForEach: public Stmt
{
    NODE_FIELDS(Stmt)
    const Expr* lval;
    const Expr* iter;
    const StmtScope* br;
};

struct StmtWhile: public Stmt
{
    NODE_FIELDS(Stmt)
    const Expr* cnd;
    const StmtScope* br;
};

struct StmtAssign: public Stmt
{
    NODE_FIELDS(Stmt)
    const Token* op;
    const Expr *lval, *rval;
};

struct StmtReturn: public Stmt
{
    NODE_FIELDS(Stmt)
    const Expr* expr;
};

struct StmtEnum: public Stmt
{
    struct Pair
    {
        const Token* symbol;
        const Expr* expr;
    };

    NODE_FIELDS(Stmt);
    const Token* symbol;
    const Type* type;
    std::vector<Pair> pairs;
};

struct StmtModule: public Stmt
{
    NODE_FIELDS(Stmt);
    const Token* symbol;
    std::vector<const Stmt*> scope;
};

struct StmtImport: public Stmt
{
    NODE_FIELDS(Stmt);

    enum class TailKind
    {
        IMPORT,          // a::b
        IMPORT_ALL,      // a::*
        IMPORT_COMPOUND, // a::{b...}
    } kind;

    std::vector<const Token*> path;
    std::vector<const Token*> tail;
};

struct StmtFunctionDecl: public Stmt
{
    NODE_FIELDS(Stmt);

    const Token* name;
    const Type* ret;
    std::vector<const Parameter*> parms;
    const StmtScope* scope;
};

struct StmtStructDecl: public Stmt
{
    NODE_FIELDS(Stmt);

    const Token* name;
    std::vector<const Stmt*> scope;
};

struct StmtTypeDecl: public Stmt
{
    NODE_FIELDS(Stmt);
    const Token* symbol;
    const Type* type;
};

struct StmtUsing: public Stmt
{
    NODE_FIELDS(Stmt);
    const Path* sp;
    const StmtScope* scope;
};

struct StmtEmpty: public Stmt
{
    NODE_FIELDS(Stmt)
};

struct StmtExpr: public Stmt
{
    NODE_FIELDS(Stmt)
    const Expr* expr;
};

struct TypeBuiltin: public Type
{
    NODE_FIELDS(Type);
    const Token* tok;
};

struct TypeArray: public Type
{
    NODE_FIELDS(Type);
    const Type* type;
};

struct TypeDict: public Type
{
    NODE_FIELDS(Type);
    const Type *key, *val;
};

struct TypeFunc: public Type
{
    NODE_FIELDS(Type);
    const Type* ret;
    std::vector<const Parameter*> params;
};

#undef NODE_FIELDS

bool isLValue(const Expr* expr) noexcept;

} // namespace ast

using SyntaxTree = std::vector<const ast::Stmt*>;

namespace debug {

[[nodiscard]] std::string to_string(const SyntaxTree& ast);

}
} // namespace via
