// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_AST_H
#define _VIA_AST_H

#include "object.h"
#include "token.h"
#include "ast_base.h"

#define VIA_AST_NODE_OVERRIDE_TO_STRING   std::string to_string(u32&) override;
#define VIA_AST_NODE_OVERRIDE_CLONE(type) type clone() override;

#define VIA_EXPR_NODE_OVERRIDE_ACCEPT     void accept(NodeVisitor&, u32) override;
#define VIA_EXPR_NODE_OVERRIDE_PRECEDENCE int precedence() const noexcept override;
#define VIA_EXPR_NODE_OVERRIDE_INFER_TYPE pTypeNode infer_type(ProgramData&) override;

#define VIA_STMT_NODE_OVERRIDE_ACCEPT void accept(NodeVisitor&) override;

#define VIA_TYPE_NODE_OVERRIDE_DECAY      void decay(NodeVisitor&, pTypeNode&) override;
#define VIA_TYPE_NODE_OVERRIDE_TO_STRINGX std::string to_string_x() override;

// ===========================================================================================
// ast.h
//
// This file declares the derived abstract syntax tree node classes,
// see `ast_base.h` for base class definitions.
//
VIA_NAMESPACE_BEGIN

struct Modifiers {
    bool is_const;

    std::string to_string() const noexcept;
};

// =========================================================================================
// Expression Nodes
//
struct LiteralNode : public ExprNode {
    using variant = std::variant<std::monostate, int, float, bool, std::string>;

    Token   value_token;
    variant value;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pExprNode);

    VIA_EXPR_NODE_OVERRIDE_ACCEPT;
    VIA_EXPR_NODE_OVERRIDE_INFER_TYPE;

    LiteralNode(Token value_token, variant value)
        : value_token(value_token),
          value(value) {
        this->begin = this->value_token.position;
        this->end   = this->value_token.position + value_token.lexeme.length();
    }
};

struct SymbolNode : public ExprNode {
    Token identifier;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pExprNode);

    VIA_EXPR_NODE_OVERRIDE_ACCEPT;
    VIA_EXPR_NODE_OVERRIDE_INFER_TYPE;

    SymbolNode(Token identifier)
        : identifier(identifier) {
        this->begin = this->identifier.position;
        this->end   = this->identifier.position + identifier.lexeme.length();
    }
};

struct UnaryNode : public ExprNode {
    pExprNode expression;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pExprNode);

    VIA_EXPR_NODE_OVERRIDE_ACCEPT;
    VIA_EXPR_NODE_OVERRIDE_INFER_TYPE;

    UnaryNode(pExprNode expression)
        : expression(std::move(expression)) {
        this->begin = this->expression->begin - 1; // Account for '-'
        this->end   = this->expression->end;
    }
};

struct GroupNode : public ExprNode {
    pExprNode expression;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pExprNode);

    VIA_EXPR_NODE_OVERRIDE_ACCEPT;
    VIA_EXPR_NODE_OVERRIDE_INFER_TYPE;
    VIA_EXPR_NODE_OVERRIDE_PRECEDENCE;

    GroupNode(pExprNode expression)
        : expression(std::move(expression)) {
        this->begin = this->expression->begin - 1; // Account for '('
        this->end   = this->expression->end + 1;   // Account for ')'
    }
};

struct CallNode : public ExprNode {
    using Arguments = std::vector<pExprNode>;

    pExprNode callee;
    Arguments arguments;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pExprNode);

    VIA_EXPR_NODE_OVERRIDE_ACCEPT;
    VIA_EXPR_NODE_OVERRIDE_INFER_TYPE;

    CallNode(pExprNode callee, Arguments arguments)
        : callee(std::move(callee)),
          arguments(std::move(arguments)) {

        pExprNode& last_arg = this->arguments.back();

        this->begin = this->callee->begin;
        this->end   = last_arg->end + 1; // Account for ')'
    }
};

struct IndexNode : public ExprNode {
    pExprNode object;
    pExprNode index;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pExprNode);

    VIA_EXPR_NODE_OVERRIDE_ACCEPT;
    VIA_EXPR_NODE_OVERRIDE_INFER_TYPE;

    IndexNode(pExprNode object, pExprNode index)
        : object(std::move(object)),
          index(std::move(index)) {
        this->begin = this->object->begin;
        this->end   = this->index->end;
    }
};

struct BinaryNode : public ExprNode {
    Token     op;
    pExprNode lhs_expression;
    pExprNode rhs_expression;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pExprNode);

    VIA_EXPR_NODE_OVERRIDE_ACCEPT;
    VIA_EXPR_NODE_OVERRIDE_INFER_TYPE;

    BinaryNode(Token op, pExprNode lhs, pExprNode rhs)
        : op(op),
          lhs_expression(std::move(lhs)),
          rhs_expression(std::move(rhs)) {
        this->begin = this->lhs_expression->begin;
        this->end   = this->rhs_expression->end;
    }
};

struct TypeCastNode : public ExprNode {
    pExprNode expression;
    pTypeNode type;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pExprNode);

    VIA_EXPR_NODE_OVERRIDE_ACCEPT;
    VIA_EXPR_NODE_OVERRIDE_INFER_TYPE;

    TypeCastNode(pExprNode expression, pTypeNode type)
        : expression(std::move(expression)),
          type(std::move(type)) {
        this->begin = this->expression->begin;
        this->end   = this->expression->end;
    }
};

// =========================================================================================
// Type Nodes
//
struct AutoNode : public TypeNode {
    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pTypeNode);

    VIA_TYPE_NODE_OVERRIDE_DECAY;
    VIA_TYPE_NODE_OVERRIDE_TO_STRINGX;
};

struct PrimitiveNode : public TypeNode {
    Token     identifier;
    ValueType type;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pTypeNode);

    VIA_TYPE_NODE_OVERRIDE_TO_STRINGX;

    PrimitiveNode(Token id, ValueType valty)
        : identifier(id),
          type(valty) {}
};

struct GenericNode : public TypeNode {
    using Generics = std::vector<pTypeNode>;
    Token     identifier;
    Generics  generics;
    Modifiers modifiers;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pTypeNode);

    VIA_TYPE_NODE_OVERRIDE_DECAY;
    VIA_TYPE_NODE_OVERRIDE_TO_STRINGX;

    GenericNode(Token id, Generics gens, Modifiers modifiers)
        : identifier(id),
          generics(std::move(gens)),
          modifiers(modifiers) {}
};

struct UnionNode : public TypeNode {
    pTypeNode lhs;
    pTypeNode rhs;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pTypeNode);

    VIA_TYPE_NODE_OVERRIDE_DECAY;
    VIA_TYPE_NODE_OVERRIDE_TO_STRINGX;

    UnionNode(pTypeNode lhs, pTypeNode rhs)
        : lhs(std::move(lhs)),
          rhs(std::move(rhs)) {}
};

struct FunctionTypeNode : public TypeNode {
    using Parameters = std::vector<pTypeNode>;
    Parameters parameters;
    pTypeNode  returns;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pTypeNode);

    VIA_TYPE_NODE_OVERRIDE_DECAY;
    VIA_TYPE_NODE_OVERRIDE_TO_STRINGX;

    FunctionTypeNode(Parameters args, pTypeNode rets)
        : parameters(std::move(args)),
          returns(std::move(rets)) {}
};

struct AggregateNode : public TypeNode {
    using Fields = std::unordered_map<std::string, pTypeNode>;
    Fields fields;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pTypeNode);

    VIA_TYPE_NODE_OVERRIDE_DECAY;
    VIA_TYPE_NODE_OVERRIDE_TO_STRINGX;

    pTypeNode get_field(const std::string&);

    AggregateNode(Fields fields)
        : fields(std::move(fields)) {}
};

// =========================================================================================
// Statement Nodes
//
struct DeclarationNode : public StmtNode {
    bool      is_global;
    Modifiers modifiers;
    Token     identifier;
    pExprNode value_expression;
    pTypeNode type;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pStmtNode);

    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    DeclarationNode(
        bool is_global, Modifiers modifiers, Token identifier, pExprNode value, pTypeNode type
    )
        : is_global(is_global),
          modifiers(modifiers),
          identifier(identifier),
          value_expression(std::move(value)),
          type(std::move(type)) {}
};

struct ScopeNode : public StmtNode {
    using Statements = std::vector<pStmtNode>;
    Statements statements;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pStmtNode);

    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    ScopeNode(Statements statements)
        : statements(std::move(statements)) {}
};

struct FunctionNode : public StmtNode {
    struct ParameterNode {
        Token     identifier;
        Modifiers modifiers;
        pTypeNode type;

        ParameterNode(Token identifier, Modifiers modifiers, pTypeNode type)
            : identifier(identifier),
              modifiers(modifiers),
              type(std::move(type)) {}
    };

    using Parameters = std::vector<ParameterNode>;

    struct StackNode {
        bool is_global;

        size_t upvalues;

        Modifiers  modifiers;
        Token      identifier;
        Parameters parameters;

        StackNode(
            bool       is_global,
            size_t     upvalues,
            Modifiers  modifiers,
            Token      identifier,
            Parameters parameters
        )
            : is_global(is_global),
              upvalues(upvalues),
              modifiers(modifiers),
              identifier(identifier),
              parameters(std::move(parameters)) {}
    };

    bool       is_global;
    Modifiers  modifiers;
    Token      identifier;
    pStmtNode  body;
    pTypeNode  returns;
    Parameters parameters;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pStmtNode);

    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    FunctionNode(
        bool       is_global,
        Modifiers  modifiers,
        Token      identifier,
        pStmtNode  body,
        pTypeNode  returns,
        Parameters parameters
    )
        : is_global(is_global),
          modifiers(modifiers),
          identifier(identifier),
          body(std::move(body)),
          returns(std::move(returns)),
          parameters(std::move(parameters)) {}
};

struct AssignNode : public StmtNode {
    pExprNode assignee;
    Token     augmentation_operator;
    pExprNode value;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pStmtNode);

    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    AssignNode(pExprNode assignee, Token augment, pExprNode value)
        : assignee(std::move(assignee)),
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

    using ElseIfNodes = std::vector<ElseIfNode>;
    using ElseNode    = std::optional<pStmtNode>;

    pExprNode   condition;
    pStmtNode   scope;
    ElseNode    else_node;
    ElseIfNodes elseif_nodes;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pStmtNode);

    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    IfNode(pExprNode condition, pStmtNode scope, ElseNode else_node, ElseIfNodes elseif_nodes)
        : condition(std::move(condition)),
          scope(std::move(scope)),
          else_node(std::move(else_node)),
          elseif_nodes(std::move(elseif_nodes)) {}
};

struct WhileNode : public StmtNode {
    pExprNode condition;
    pStmtNode body;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pStmtNode);

    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    WhileNode(pExprNode condition, pStmtNode body)
        : condition(std::move(condition)),
          body(std::move(body)) {}
};

struct ExprStmtNode : public StmtNode {
    pExprNode expression;

    VIA_AST_NODE_OVERRIDE_TO_STRING;
    VIA_AST_NODE_OVERRIDE_CLONE(pStmtNode);

    VIA_STMT_NODE_OVERRIDE_ACCEPT;

    ExprStmtNode(pExprNode expression)
        : expression(std::move(expression)) {}
};

struct AbstractSyntaxTree {
    std::vector<pStmtNode> statements;
};

VIA_NAMESPACE_END

#endif
