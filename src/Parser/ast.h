// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "token.h"
#include "ast_base.h"

namespace via {

struct Modifiers {
    bool is_const;
};

// ============================ |
//       Expression Nodes       |
// ============================ |
struct LiteralNode : public ExprNode {
    using variant = std::variant<std::monostate, int, float, bool, std::string>;

    Token value_token;
    variant value;
    std::string to_string() override;
    void accept(NodeVisitor &, U32) override;

    LiteralNode(Token val_token, const variant &val)
        : value_token(val_token)
        , value(val)
    {
    }
};

struct VariableNode : public ExprNode {
    Token identifier;
    std::string to_string() override;
    void accept(NodeVisitor &, U32) override;

    VariableNode(Token identifier)
        : identifier(identifier)
    {
    }
};

struct UnaryNode : public ExprNode {
    pExprNode expression;
    std::string to_string() override;
    void accept(NodeVisitor &, U32) override;

    UnaryNode(pExprNode expression)
        : expression(std::move(expression))
    {
    }
};

struct GroupNode : public ExprNode {
    pExprNode expression;
    std::string to_string() override;
    int precedence() const noexcept override;

    GroupNode(pExprNode expression)
        : expression(std::move(expression))
    {
    }
};

struct CallNode : public ExprNode {
    pExprNode callee;
    std::vector<pExprNode> arguments;
    std::string to_string() override;
    void accept(NodeVisitor &, U32) override;

    CallNode(pExprNode callee, std::vector<pExprNode> arguments)
        : callee(std::move(callee))
        , arguments(std::move(arguments))
    {
    }
};

struct IndexNode : public ExprNode {
    pExprNode object;
    pExprNode index;
    std::string to_string() override;
    void accept(NodeVisitor &, U32) override;

    IndexNode(pExprNode object, pExprNode index)
        : object(std::move(object))
        , index(std::move(index))
    {
    }
};

struct BinaryNode : public ExprNode {
    Token op;
    pExprNode lhs_expression;
    pExprNode rhs_expression;
    std::string to_string() override;
    void accept(NodeVisitor &, U32) override;

    BinaryNode(Token op, pExprNode lhs, pExprNode rhs)
        : op(op)
        , lhs_expression(std::move(lhs))
        , rhs_expression(std::move(rhs))
    {
    }
};

// ============================ |
//       Statement Nodes        |
// ============================ |
struct DeclarationNode : public StmtNode {
    bool is_global;
    Modifiers modifiers;
    Token identifier;
    pExprNode value_expression;
    std::string to_string() override;
    void accept(NodeVisitor &) override;

    DeclarationNode(
        bool is_global,
        Modifiers modifiers,
        Token identifier,
        pExprNode value_expression
    )
        : is_global(is_global)
        , modifiers(modifiers)
        , identifier(identifier)
        , value_expression(std::move(value_expression))
    {
    }
};

struct ExprStmtNode : public StmtNode {
    pExprNode expression;
    std::string to_string() override;
    void accept(NodeVisitor &) override;

    ExprStmtNode(pExprNode expression)
        : expression(std::move(expression))
    {
    }
};

struct AbstractSyntaxTree {
    std::vector<pStmtNode> statements;
};

} // namespace via
