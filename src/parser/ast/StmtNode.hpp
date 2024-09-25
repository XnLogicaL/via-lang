#pragma once

#include <variant>
#include <vector>
#include <functional>

#include "Literals.hpp"
#include "../../lexer/token.hpp"

struct ExprNode;
struct StmtNode;
struct IfStmtNode;

struct StmtExitNode { Token val = NULL_TOKEN; ExprNode* expr; };
struct ScopeNode { Token val = NULL_TOKEN; std::vector<StmtNode*> stmts; };

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
    std::vector<ParamNode> params;
    ScopeNode* body;
    Token val = NULL_TOKEN;
    std::function<void(std::vector<ExprNode*>)> c_hook;
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
    Token op;
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
        FuncNode*,
        NilNode*
    > node;
    Token val = NULL_TOKEN;

    size_t get_line()
    {
        return std::visit([](const auto& _expr) {
            return _expr->val.line;
        }, this->node);
    }
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
    Token val = NULL_TOKEN;
};