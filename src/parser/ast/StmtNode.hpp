#pragma once

#include <variant>
#include <vector>

#include "Literals.hpp"
#include "../../lexer/token.hpp"

struct ExprNode;
struct StmtNode;
struct IfStmtNode;

struct StmtExitNode { ExprNode* expr; };
struct ScopeNode { std::vector<StmtNode*> stmts; };

struct LocalDeclNode
{
    Token val = NULL_TOKEN;
    Token ident;
    ExprNode* expr;
    bool is_const;
};

struct StmtAssignNode
{
    Token val = NULL_TOKEN;
    Token ident;
    ExprNode* expr;
};

struct FuncCallNode
{
    Token val = NULL_TOKEN;
    Token ident;
    std::vector<ExprNode*> args;
};

struct FuncNode {
    Token ident;
    std::vector<Token> params;
    ScopeNode* body;
    Token val = NULL_TOKEN;
};

struct StmtNode {
    std::variant<
        LocalDeclNode*,
        StmtAssignNode*,
        ScopeNode*,
        IfStmtNode*,
        FuncCallNode*,
        ExprNode*,
        StmtExitNode*
    > stmt;
};

/* Expr */
struct BinExprNode
{
    Token val = NULL_TOKEN;
    TokenType op;
    ExprNode* lhs;
    ExprNode* rhs;
};

struct ParenExprNode
{
    Token val = NULL_TOKEN;
    ExprNode* expr;
};

struct ExprNode {
    std::variant<
        IntLitNode*,
        BoolLitNode*,
        StringLitNode*,
        IdentNode*,
        FuncCallNode*,
        ParenExprNode*,
        BinExprNode*,
        FuncNode*
    > node;
};

/* IfStmts */
struct ElifPredNode
{
    ExprNode* cond;
    ScopeNode* then_scope;
};

struct IfPredNode
{
    ExprNode* cond;
    ScopeNode* then_scope;
    ScopeNode* else_scope;
    std::vector<ElifPredNode*> elif_nodes;
};

struct IfStmtNode
{
    IfPredNode* if_pred;
};