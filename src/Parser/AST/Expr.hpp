#pragma once

#ifndef via_expr_hpp
#define via_expr_hpp

#include "../../common.hpp"
#include "../../Lexer/token.hpp"

struct BinExpr;
struct LitExpr;
struct UnExpr;
struct GroupExpr;
struct RefExpr;
struct LambdaExpr;
struct CallExpr;
struct NotExpr;
struct OrExpr;
struct AndExpr;

typedef std::variant<
    BinExpr,
    LitExpr,
    UnExpr,
    GroupExpr,
    RefExpr,
    LambdaExpr,
    CallExpr,
    NotExpr,
    OrExpr,
    AndExpr
> Expr;

struct LitExpr
{
    Token lit;
};

struct UnExpr
{
    Expr* expr;
};

struct GroupExpr
{
    Expr* expr;
};

struct RefExpr
{
    Token ident;
};

struct BinExpr
{
    Token op;
    Expr* lhs;
    Expr* rhs;
};

struct CallExpr
{
    Token ident;
    std::vector<Expr> args;
};

struct NotExpr
{
    Expr* expr;
};

struct AndExpr
{
    Expr* lhs;
    Expr* rhs;
};

struct OrExpr
{
    Expr* lhs;
    Expr* rhs;
};

#endif