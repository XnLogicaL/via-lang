// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_AST_H
#define _VIA_AST_H

#include "token.h"
#include "ast_base.h"

#define VIA_AST_NODE_OVERRIDE_TO_STRING std::string to_string(U32&) override;
#define VIA_EXPR_NODE_OVERRIDE_ACCEPT   void accept(NodeVisitor&, U32) override;
#define VIA_STMT_NODE_OVERRIDE_ACCEPT   void accept(NodeVisitor&) override;

// ================================================================ |
// File ast.h: Abstract syntax tree declarations.                   |
// ================================================================ |
// This file declares the derived abstract syntax tree node classes,
// see `ast_base.h` for base class definitions.
// ================================================================ |
VIA_NAMESPACE_BEGIN

struct Modifiers {
    bool is_const;

    std::string to_string() const noexcept;
};

// ============================ |
//       Expression Nodes       |
// ============================ |

struct LiteralNode : public ExprNode {
    using variant = std::variant<std::monostate, int, float, bool, std::string>;

    Token   value_token;
    variant value;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_EXPR_NODE_OVERRIDE_ACCEPT;

    LiteralNode(Token value_token, variant value)
        : value_token(value_token),
          value(value) {}
};

struct SymbolNode : public ExprNode {
    Token identifier;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_EXPR_NODE_OVERRIDE_ACCEPT;

    SymbolNode(Token identifier)
        : identifier(identifier) {}
};

struct UnaryNode : public ExprNode {
    pExprNode expression;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_EXPR_NODE_OVERRIDE_ACCEPT;

    UnaryNode(pExprNode expression)
        : expression(std::move(expression)) {}
};

struct GroupNode : public ExprNode {
    pExprNode expression;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_EXPR_NODE_OVERRIDE_ACCEPT;

    GroupNode(pExprNode expression)
        : expression(std::move(expression)) {}
};

struct CallNode : public ExprNode {
    pExprNode              callee;
    std::vector<pExprNode> arguments;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_EXPR_NODE_OVERRIDE_ACCEPT;

    CallNode(pExprNode callee, std::vector<pExprNode> arguments)
        : callee(std::move(callee)),
          arguments(std::move(arguments)) {}
};

struct IndexNode : public ExprNode {
    pExprNode object;
    pExprNode index;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_EXPR_NODE_OVERRIDE_ACCEPT;

    IndexNode(pExprNode object, pExprNode index)
        : object(std::move(object)),
          index(std::move(index)) {}
};

struct BinaryNode : public ExprNode {
    Token     op;
    pExprNode lhs_expression;
    pExprNode rhs_expression;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_EXPR_NODE_OVERRIDE_ACCEPT;

    BinaryNode(Token op, pExprNode lhs, pExprNode rhs)
        : op(op),
          lhs_expression(std::move(lhs)),
          rhs_expression(std::move(rhs)) {}
};

// ============================ |
//       Statement Nodes        |
// ============================ |

struct DeclarationNode : public StmtNode {
    bool      is_global;
    Modifiers modifiers;
    Token     identifier;
    pExprNode value_expression;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    DeclarationNode(bool is_global, Modifiers modifiers, Token identifier, pExprNode value)
        : is_global(is_global),
          modifiers(modifiers),
          identifier(identifier),
          value_expression(std::move(value)) {}
};

struct ScopeNode : public StmtNode {
    std::vector<pStmtNode> statements;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    ScopeNode(std::vector<pStmtNode> statements)
        : statements(std::move(statements)) {}
};

struct FunctionNode : public StmtNode {
    struct ParameterNode {
        Token identifier;

        ParameterNode(Token identifier)
            : identifier(identifier) {}
    };

    struct StackNode {
        bool                       is_global;
        Modifiers                  modifiers;
        Token                      identifier;
        std::vector<ParameterNode> parameters;

        StackNode(bool is_global, Modifiers modifiers, Token identifier,
            std::vector<ParameterNode> parameters)
            : is_global(is_global),
              modifiers(modifiers),
              identifier(identifier),
              parameters(parameters) {}
    };

    bool                       is_global;
    Modifiers                  modifiers;
    Token                      identifier;
    pStmtNode                  body;
    std::vector<ParameterNode> parameters;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    FunctionNode(bool is_global, Modifiers modifiers, Token identifier, pStmtNode body,
        std::vector<ParameterNode> parameters)
        : is_global(is_global),
          modifiers(modifiers),
          identifier(identifier),
          body(std::move(body)),
          parameters(parameters) {}
};

struct AssignNode : public StmtNode {
    Token     identifier;
    Token     augmentation_operator;
    pExprNode value;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    AssignNode(Token identifier, Token augment, pExprNode value)
        : identifier(identifier),
          augmentation_operator(augment),
          value(std::move(value)) {}
};

struct IfNode : public StmtNode {
    struct ElseIfNode {
        pExprNode condition;
        pStmtNode scope;

        ElseIfNode(pExprNode condition, pStmtNode scope)
            : condition(std::move(condition)),
              scope(std::move(scope)) {}
    };

    pExprNode                condition;
    pStmtNode                scope;
    std::optional<pStmtNode> else_node;
    std::vector<ElseIfNode>  elseif_nodes;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    IfNode(pExprNode condition, pStmtNode scope, std::optional<pStmtNode> else_node,
        std::vector<ElseIfNode> elseif_nodes)
        : condition(std::move(condition)),
          scope(std::move(scope)),
          else_node(std::move(else_node)),
          elseif_nodes(std::move(elseif_nodes)) {}
};

struct WhileNode : public StmtNode {
    pExprNode condition;
    pStmtNode body;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    WhileNode(pExprNode condition, pStmtNode body)
        : condition(std::move(condition)),
          body(std::move(body)) {}
};

struct ExprStmtNode : public StmtNode {
    pExprNode expression;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    ExprStmtNode(pExprNode expression)
        : expression(std::move(expression)) {}
};

struct AbstractSyntaxTree {
    std::vector<pStmtNode> statements;
};

VIA_NAMESPACE_END

#endif
