// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_AST_H
#define _VIA_AST_H

#include "object.h"
#include "token.h"
#include "ast-base.h"

// ===========================================================================================
// ast.h
//
// This file declares the derived abstract syntax tree node classes,
// see `ast-base.h` for base class definitions.
//
VIA_NAMESPACE_BEGIN

struct Modifiers {
    bool is_const;

    std::string to_string() const;
};

// =========================================================================================
// Expression Nodes
//
struct LiteralNode : public ExprNode {
    using variant = std::variant<std::monostate, int, float, bool, std::string>;

    Token   value_token;
    variant value;

    std::string to_string(uint32_t&) override;

    pExprNode clone() override;
    pTypeNode infer_type(ProgramData&) override;

    void accept(NodeVisitor&, uint32_t) override;

    LiteralNode(Token value_token, variant value)
        : value_token(value_token),
          value(value) {
        this->begin = this->value_token.position;
        this->end   = this->value_token.position + value_token.lexeme.length();
    }
};

struct SymbolNode : public ExprNode {
    Token identifier;

    std::string to_string(uint32_t&) override;

    pExprNode clone() override;
    pTypeNode infer_type(ProgramData&) override;

    void accept(NodeVisitor&, uint32_t) override;

    SymbolNode(Token identifier)
        : identifier(identifier) {
        this->begin = this->identifier.position;
        this->end   = this->identifier.position + identifier.lexeme.length();
    }
};

struct UnaryNode : public ExprNode {
    pExprNode expression;

    std::string to_string(uint32_t&) override;

    pExprNode clone() override;
    pTypeNode infer_type(ProgramData&) override;

    void accept(NodeVisitor&, uint32_t) override;

    UnaryNode(pExprNode expression)
        : expression(std::move(expression)) {
        this->begin = this->expression->begin - 1; // Account for '-'
        this->end   = this->expression->end;
    }
};

struct GroupNode : public ExprNode {
    pExprNode expression;

    std::string to_string(uint32_t&) override;

    pExprNode clone() override;
    pTypeNode infer_type(ProgramData&) override;

    void accept(NodeVisitor&, uint32_t) override;
    int  precedence() const override;

    GroupNode(pExprNode expression)
        : expression(std::move(expression)) {
        this->begin = this->expression->begin - 1; // Account for '('
        this->end   = this->expression->end + 1;   // Account for ')'
    }
};

struct CallNode : public ExprNode {
    using argument_vector = std::vector<pExprNode>;

    pExprNode       callee;
    argument_vector arguments;

    std::string to_string(uint32_t&) override;

    pExprNode clone() override;
    pTypeNode infer_type(ProgramData&) override;

    void accept(NodeVisitor&, uint32_t) override;

    CallNode(pExprNode callee, argument_vector arguments)
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

    std::string to_string(uint32_t&) override;

    pExprNode clone() override;
    pTypeNode infer_type(ProgramData&) override;

    void accept(NodeVisitor&, uint32_t) override;

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

    std::string to_string(uint32_t&) override;

    pExprNode clone() override;
    pTypeNode infer_type(ProgramData&) override;

    void accept(NodeVisitor&, uint32_t) override;

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

    std::string to_string(uint32_t&) override;

    pExprNode clone() override;
    pTypeNode infer_type(ProgramData&) override;

    void accept(NodeVisitor&, uint32_t) override;

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
    std::string to_string(uint32_t&) override;
    std::string to_string_x() override;

    pTypeNode clone() override;

    void decay(NodeVisitor&, pTypeNode&) override;
};

struct PrimitiveNode : public TypeNode {
    Token     identifier;
    ValueType type;

    std::string to_string(uint32_t&) override;
    std::string to_string_x() override;

    pTypeNode clone() override;

    PrimitiveNode(Token id, ValueType valty)
        : identifier(id),
          type(valty) {}
};

struct GenericNode : public TypeNode {
    using Generics = std::vector<pTypeNode>;
    Token     identifier;
    Generics  generics;
    Modifiers modifiers;

    std::string to_string(uint32_t&) override;
    std::string to_string_x() override;

    pTypeNode clone() override;

    void decay(NodeVisitor&, pTypeNode&) override;

    GenericNode(Token id, Generics gens, Modifiers modifiers)
        : identifier(id),
          generics(std::move(gens)),
          modifiers(modifiers) {}
};

struct UnionNode : public TypeNode {
    pTypeNode lhs;
    pTypeNode rhs;

    std::string to_string(uint32_t&) override;
    std::string to_string_x() override;

    pTypeNode clone() override;

    void decay(NodeVisitor&, pTypeNode&) override;

    UnionNode(pTypeNode lhs, pTypeNode rhs)
        : lhs(std::move(lhs)),
          rhs(std::move(rhs)) {}
};

struct FunctionTypeNode : public TypeNode {
    using Parameters = std::vector<pTypeNode>;
    Parameters parameters;
    pTypeNode  returns;

    std::string to_string(uint32_t&) override;
    std::string to_string_x() override;

    pTypeNode clone() override;

    void decay(NodeVisitor&, pTypeNode&) override;

    FunctionTypeNode(Parameters args, pTypeNode rets)
        : parameters(std::move(args)),
          returns(std::move(rets)) {}
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

    std::string to_string(uint32_t&) override;

    pStmtNode clone() override;

    void accept(NodeVisitor&) override;

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

    std::string to_string(uint32_t&) override;

    pStmtNode clone() override;

    void accept(NodeVisitor&) override;

    ScopeNode(Statements statements)
        : statements(std::move(statements)) {}
};

struct ParameterNode {
    Token     identifier;
    Modifiers modifiers;
    pTypeNode type;

    ParameterNode(Token identifier, Modifiers modifiers, pTypeNode type)
        : identifier(identifier),
          modifiers(modifiers),
          type(std::move(type)) {}
};

struct FunctionNode : public StmtNode {
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

    std::string to_string(uint32_t&) override;

    pStmtNode clone() override;

    void accept(NodeVisitor&) override;

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

struct ConstructorNode : public StmtNode {
    using Parameters = std::vector<ParameterNode>;

    pStmtNode  body;
    Parameters parameters;

    std::string to_string(uint32_t&) override;

    pStmtNode clone() override;

    void accept(NodeVisitor&) override;

    ConstructorNode(pStmtNode body, Parameters parameters)
        : body(std::move(body)),
          parameters(std::move(parameters)) {}
};

struct DestructorNode : public StmtNode {
    pStmtNode body;

    std::string to_string(uint32_t&) override;
    pStmtNode   clone() override;

    void accept(NodeVisitor&) override;

    DestructorNode(pStmtNode body)
        : body(std::move(body)) {}
};

struct MemberNode : public StmtNode {
    Token     identifier;
    pTypeNode type;
    pExprNode initializer;

    std::string to_string(uint32_t&) override;
    pStmtNode   clone() override;

    void accept(NodeVisitor&) override;

    MemberNode(Token id, pTypeNode ty, pExprNode init)
        : identifier(id),
          type(std::move(ty)),
          initializer(std::move(init)) {}
};

struct AssignNode : public StmtNode {
    pExprNode assignee;
    Token     augmentation_operator;
    pExprNode value;

    std::string to_string(uint32_t&) override;
    pStmtNode   clone() override;

    void accept(NodeVisitor&) override;

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

    std::string to_string(uint32_t&) override;
    pStmtNode   clone() override;

    void accept(NodeVisitor&) override;

    IfNode(pExprNode condition, pStmtNode scope, ElseNode else_node, ElseIfNodes elseif_nodes)
        : condition(std::move(condition)),
          scope(std::move(scope)),
          else_node(std::move(else_node)),
          elseif_nodes(std::move(elseif_nodes)) {}
};

struct ReturnNode : public StmtNode {
    pExprNode expression;

    std::string to_string(uint32_t&) override;
    pStmtNode   clone() override;

    void accept(NodeVisitor&) override;

    ReturnNode(pExprNode expression)
        : expression(std::move(expression)) {}
};

struct BreakNode : public StmtNode {
    Token token;

    std::string to_string(uint32_t&) override;
    pStmtNode   clone() override;

    void accept(NodeVisitor&) override;

    BreakNode(Token tok)
        : token(tok) {}
};

struct ContinueNode : public StmtNode {
    Token token;

    std::string to_string(uint32_t&) override;
    pStmtNode   clone() override;

    void accept(NodeVisitor&) override;

    ContinueNode(Token tok)
        : token(tok) {}
};

struct WhileNode : public StmtNode {
    pExprNode condition;
    pStmtNode body;

    std::string to_string(uint32_t&) override;
    pStmtNode   clone() override;

    void accept(NodeVisitor&) override;

    WhileNode(pExprNode condition, pStmtNode body)
        : condition(std::move(condition)),
          body(std::move(body)) {}
};

struct ExprStmtNode : public StmtNode {
    pExprNode expression;

    std::string to_string(uint32_t&) override;

    pStmtNode clone() override;

    void accept(NodeVisitor&) override;

    ExprStmtNode(pExprNode expression)
        : expression(std::move(expression)) {}
};

class AbstractSyntaxTree {
public:
    std::vector<pStmtNode> statements;
};

VIA_NAMESPACE_END

#endif
