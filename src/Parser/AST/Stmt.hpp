#pragma once

#ifndef via_stmt_hpp
#define via_stmt_hpp

#include "../../common.hpp"
#include "../../Lexer/token.hpp"

#include "Expr.hpp"

struct SwitchStmt;
struct IfStmt;
struct WhileStmt;
struct ForStmt;
struct CallStmt;
struct IndexStmt;
struct IndexCallStmt;
struct IndexAssignStmt;
struct DeclStmt;
struct AssignStmt;
struct FunDeclStmt;
struct ClassDeclStmt;
struct ModuleDeclStmt;
struct ImportStmt;
struct ExportStmt;
struct DerefStmt;
struct RefStmt;
struct BreakStmt;
struct ContinueStmt;
struct ReturnStmt;
struct BlockStmt;

typedef std::variant <
    SwitchStmt,
    IfStmt,
    WhileStmt,
    ForStmt,
    CallStmt,
    IndexStmt,
    IndexCallStmt,
    IndexAssignStmt,
    DeclStmt,
    AssignStmt,
    FunDeclStmt,
    ClassDeclStmt,
    ModuleDeclStmt,
    ImportStmt,
    ExportStmt,
    DerefStmt,
    RefStmt,
    BreakStmt,
    ContinueStmt,
    ReturnStmt,
    BlockStmt
> Stmt;

typedef std::variant<IndexStmt, IndexAssignStmt, IndexCallStmt> _IndexStmt;
typedef std::variant<ModuleDeclStmt, FunDeclStmt, DeclStmt> _DeclStmt;

struct ContinueStmt {};
struct BreakStmt {};

struct BlockStmt
{
    std::vector<Stmt> stmts;
};

struct ElifStmt
{
    Expr cond;
    BlockStmt then_block;
};

struct IfStmt
{
    Expr cond;
    BlockStmt then_block;
    BlockStmt else_block;
    std::vector<ElifStmt> elif_stmts;
};

struct WhileStmt
{
    Expr cond;
    BlockStmt do_block;
};

struct ForStmt
{
    Token k_ident;
    Token v_ident;
    Expr iterator;
    BlockStmt do_block;
};

struct CallStmt
{
    Token ident;
    std::vector<Expr> args;
};

struct IndexStmt
{
    Token ident;
    Expr key;
};

struct IndexCallStmt
{
    Token ident;
    Expr key;
    std::vector<Expr> args;
};

struct IndexAssignStmt
{
    Token ident;
    Expr key;
    Expr value;
};

struct DeclStmt
{
    bool is_cnst;
    bool is_glb;

    Token ident;
    Expr value;
};

struct AssignStmt
{
    Token ident;
    Expr value;
};

struct FunDeclStmt
{
    bool is_cnst;
    bool is_glb;

    Token ident;
    std::vector<Token> params;
    BlockStmt fun_scope;
};

struct ClassDeclStmt
{
    Token ident;

    FunDeclStmt constructor;
    FunDeclStmt destructor;
    
    std::vector<DeclStmt> attribs;
    std::vector<FunDeclStmt> methods;
};

struct ModuleDeclStmt
{
    Token ident;
};

struct ImportStmt
{
    Token path;
    Token ident;
};

struct ExportStmt
{
    Token ident;
};

struct RefStmt
{
    Expr expr;
};

struct DerefStmt
{
    Expr expr;
};

struct ReturnStmt
{
    Expr ret;
};

struct CaseStmt
{
    Expr expr;
    BlockStmt case_block;
};

struct DefaultStmt
{
    BlockStmt default_block;
};

struct SwitchStmt
{
    Expr value;
    std::vector<CaseStmt> cases;
    DefaultStmt def;
};

// Expr declarations
struct LambdaExpr
{
    std::vector<Token> params;
    BlockStmt fun_scope;
};

#endif