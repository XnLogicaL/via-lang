/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <vector>
#include <via/config.hpp>
#include "lexer/token.hpp"
#include "sema/types.hpp"
#include "source.hpp"

#define TRY_COERCE(T, a, b) (T* a = dynamic_cast<T*>(b))
#define TRY_IS(T, a) (dynamic_cast<T*>(a) != nullptr)

namespace via {
namespace ast {

struct Expr
{
    SourceLoc loc;
    virtual ~Expr() = default;
    virtual std::string to_string(size_t depth = 0) const = 0;
};

struct Stmt
{
    SourceLoc loc;
    virtual ~Stmt() = default;
    virtual std::string to_string(size_t depth = 0) const = 0;
};

struct Type
{
    SourceLoc loc;
    TypeQualifier quals;
    virtual ~Type() = default;
    virtual std::string to_string(size_t depth = 0) const = 0;
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

struct Scope
{
    std::vector<const Stmt*> stmts;
    SourceLoc loc;
    std::string to_string(size_t depth) const;
};

#undef NODE_FIELDS
#define NODE_FIELDS(CLASS)                                                               \
    ~CLASS() = default;                                                                  \
    std::string to_string(size_t depth = 0) const override;

struct ExprLiteral: public Expr
{
    NODE_FIELDS(ExprLiteral);
    const Token* tok;
};

struct ExprSymbol: public Expr
{
    NODE_FIELDS(ExprSymbol);
    const Token* symbol;
};

struct ExprDynAccess: public Expr
{
    NODE_FIELDS(ExprDynAccess);
    const Expr* root;
    const Token* index;
};

struct ExprStaticAccess: public Expr
{
    NODE_FIELDS(ExprStaticAccess);
    const Expr* root;
    const Token* index;
};

struct ExprUnary: public Expr
{
    NODE_FIELDS(ExprUnary);
    const Token* op;
    const Expr* expr;
};

struct ExprBinary: public Expr
{
    NODE_FIELDS(ExprBinary);
    const Token* op;
    const Expr *lhs, *rhs;
};

struct ExprGroup: public Expr
{
    NODE_FIELDS(ExprGroup);
    const Expr* expr;
};

struct ExprCall: public Expr
{
    NODE_FIELDS(ExprCall);
    const Expr* callee;
    std::vector<const Expr*> args;
};

struct ExprSubscript: public Expr
{
    NODE_FIELDS(ExprSubscript);
    const Expr *lhs, *rhs;
};

struct ExprCast: public Expr
{
    NODE_FIELDS(ExprCast);
    const Expr* expr;
    const Type* type;
};

struct ExprTernary: public Expr
{
    NODE_FIELDS(ExprTernary);
    const Expr *cond, *lhs, *rhs;
};

struct ExprArray: public Expr
{
    NODE_FIELDS(ExprArray);
    std::vector<const Expr*> values;
};

struct ExprTuple: public Expr
{
    NODE_FIELDS(ExprTuple);
    std::vector<const Expr*> values;
};

struct ExprLambda: public Expr
{
    NODE_FIELDS(ExprLambda);
    const Type* ret;
    std::vector<const Parameter*> parms;
    const Scope* body;
};

struct StmtVarDecl: public Stmt
{
    NODE_FIELDS(StmtVarDecl);
    const Token* decl;
    const Expr* lval;
    const Expr* rval;
    const Type* type;
};

struct StmtScope: public Stmt
{
    NODE_FIELDS(StmtScope);
    const Scope* body;
};

struct StmtIf: public Stmt
{
    struct Branch
    {
        const Expr* cond;
        const Scope* body;
    };

    NODE_FIELDS(StmtIf);
    std::vector<Branch> branches;
};

struct StmtFor: public Stmt
{
    NODE_FIELDS(StmtFor);
    const StmtVarDecl* init;
    const Expr *target, *step;
    const Scope* body;
};

struct StmtForEach: public Stmt
{
    NODE_FIELDS(StmtForEach);
    const Expr* name;
    const Expr* expr;
    const Scope* body;
};

struct StmtWhile: public Stmt
{
    NODE_FIELDS(StmtWhile);
    const Expr* cond;
    const Scope* body;
};

struct StmtAssign: public Stmt
{
    NODE_FIELDS(StmtAssign);
    const Token* op;
    const Expr *lval, *rval;
};

struct StmtReturn: public Stmt
{
    NODE_FIELDS(StmtReturn);
    const Expr* expr;
};

struct StmtEnum: public Stmt
{
    struct Pair
    {
        const Token* symbol;
        const Expr* expr;
    };

    NODE_FIELDS(StmtEnum);
    const Token* symbol;
    const Type* type;
    std::vector<Pair> pairs;
};

struct StmtImport: public Stmt
{
    NODE_FIELDS(StmtImport);
    std::vector<const Token*> path;
};

struct StmtFunctionDecl: public Stmt
{
    NODE_FIELDS(StmtFunctionDecl);
    const Token* name;
    const Type* ret;
    std::vector<const Parameter*> parms;
    const Scope* body;
};

struct StmtStructDecl: public Stmt
{
    NODE_FIELDS(StmtStructDecl);
    const Token* name;
    const Scope* body;
};

struct StmtTypeDecl: public Stmt
{
    NODE_FIELDS(StmtTypeDecl);
    const Token* symbol;
    const Type* type;
};

struct StmtEmpty: public Stmt
{
    NODE_FIELDS(StmtEmpty);
};

struct StmtExpr: public Stmt
{
    NODE_FIELDS(StmtExpr);
    const Expr* expr;
};

struct TypeBuiltin: public Type
{
    NODE_FIELDS(TypeBuiltin);
    const Token* token;
};

struct TypeArray: public Type
{
    NODE_FIELDS(TypeArray);
    const Type* type;
};

struct TypeMap: public Type
{
    NODE_FIELDS(TypeMap);
    const Type *key, *value;
};

struct TypeFunc: public Type
{
    NODE_FIELDS(TypeFunc);
    const Type* ret;
    std::vector<const Parameter*> parms;
};

#undef NODE_FIELDS

bool is_lvalue(const Expr* expr) noexcept;

} // namespace ast

using SyntaxTree = std::vector<const ast::Stmt*>;

std::string to_string(const SyntaxTree& ast);

} // namespace via
