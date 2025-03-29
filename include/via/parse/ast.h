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

// =========================================================================================
// Expression Nodes
//
struct LiteralExprNode : public ExprNode {
  using variant = std::variant<std::monostate, int, float, bool, std::string>;

  Token value_token;
  variant value;

  std::string to_string(uint32_t&) override;

  pExprNode clone() override;
  pTypeNode infer_type(TransUnitContext&) override;

  void accept(NodeVisitor&, uint32_t) override;

  LiteralExprNode(Token value_token, variant value)
    : value_token(value_token),
      value(value) {
    this->begin = this->value_token.position;
    this->end = this->value_token.position + value_token.lexeme.length();
  }
};

struct SymbolExprNode : public ExprNode {
  Token identifier;

  std::string to_string(uint32_t&) override;

  pExprNode clone() override;
  pTypeNode infer_type(TransUnitContext&) override;

  void accept(NodeVisitor&, uint32_t) override;

  SymbolExprNode(Token identifier)
    : identifier(identifier) {
    this->begin = this->identifier.position;
    this->end = this->identifier.position + identifier.lexeme.length();
  }
};

struct UnaryExprNode : public ExprNode {
  pExprNode expression;

  std::string to_string(uint32_t&) override;

  pExprNode clone() override;
  pTypeNode infer_type(TransUnitContext&) override;

  void accept(NodeVisitor&, uint32_t) override;

  UnaryExprNode(pExprNode expression)
    : expression(std::move(expression)) {
    this->begin = this->expression->begin - 1; // Account for '-'
    this->end = this->expression->end;
  }
};

struct GroupExprNode : public ExprNode {
  pExprNode expression;

  std::string to_string(uint32_t&) override;

  pExprNode clone() override;
  pTypeNode infer_type(TransUnitContext&) override;

  void accept(NodeVisitor&, uint32_t) override;
  int precedence() const override;

  GroupExprNode(pExprNode expression)
    : expression(std::move(expression)) {
    this->begin = this->expression->begin - 1; // Account for '('
    this->end = this->expression->end + 1;     // Account for ')'
  }
};

struct CallExprNode : public ExprNode {
  using argument_vector = std::vector<pExprNode>;

  pExprNode callee;
  argument_vector arguments;

  std::string to_string(uint32_t&) override;

  pExprNode clone() override;
  pTypeNode infer_type(TransUnitContext&) override;

  void accept(NodeVisitor&, uint32_t) override;

  CallExprNode(pExprNode callee, argument_vector arguments)
    : callee(std::move(callee)),
      arguments(std::move(arguments)) {

    if (!this->arguments.empty()) {
      pExprNode& last_arg = this->arguments.back();

      this->begin = this->callee->begin;
      this->end = last_arg->end + 1; // Account for ')'
    }
    else {
      this->begin = this->callee->begin;
      this->end = this->callee->end + 2; // Account for '()'
    }
  }
};

struct IndexExprNode : public ExprNode {
  pExprNode object;
  pExprNode index;

  std::string to_string(uint32_t&) override;

  pExprNode clone() override;
  pTypeNode infer_type(TransUnitContext&) override;

  void accept(NodeVisitor&, uint32_t) override;

  IndexExprNode(pExprNode object, pExprNode index)
    : object(std::move(object)),
      index(std::move(index)) {
    this->begin = this->object->begin;
    this->end = this->index->end;
  }
};

struct BinaryExprNode : public ExprNode {
  Token op;
  pExprNode lhs_expression;
  pExprNode rhs_expression;

  std::string to_string(uint32_t&) override;

  pExprNode clone() override;
  pTypeNode infer_type(TransUnitContext&) override;

  void accept(NodeVisitor&, uint32_t) override;

  BinaryExprNode(Token op, pExprNode lhs, pExprNode rhs)
    : op(op),
      lhs_expression(std::move(lhs)),
      rhs_expression(std::move(rhs)) {
    this->begin = this->lhs_expression->begin;
    this->end = this->rhs_expression->end;
  }
};

struct TypeCastExprNode : public ExprNode {
  pExprNode expression;
  pTypeNode type;

  std::string to_string(uint32_t&) override;

  pExprNode clone() override;
  pTypeNode infer_type(TransUnitContext&) override;

  void accept(NodeVisitor&, uint32_t) override;

  TypeCastExprNode(pExprNode expression, pTypeNode type)
    : expression(std::move(expression)),
      type(std::move(type)) {
    this->begin = this->expression->begin;
    this->end = this->expression->end;
  }
};

struct TableExprNode : public ExprNode {
  struct KvPair {
    pExprNode key;
    pExprNode val;
  };

  using kvpair_vector = std::vector<KvPair>;

  Token open_brace;
  Token close_brace;
  kvpair_vector pairs;

  std::string to_string(uint32_t&) override;

  pExprNode clone() override;
  pTypeNode infer_type(TransUnitContext&) override;

  void accept(NodeVisitor&, uint32_t) override;

  TableExprNode(Token open_brace, Token close_brace, kvpair_vector pairs)
    : pairs(std::move(pairs)) {
    this->begin = open_brace.position;
    this->end = close_brace.position;
  }
};

// =========================================================================================
// Type Nodes
//
struct AutoTypeNode : public TypeNode {
  std::string to_string(uint32_t&) override;
  std::string to_string_x() override;

  pTypeNode clone() override;

  void decay(NodeVisitor&, pTypeNode&) override;
};

struct PrimitiveTypeNode : public TypeNode {
  Token identifier;
  ValueType type;

  std::string to_string(uint32_t&) override;
  std::string to_string_x() override;

  pTypeNode clone() override;

  PrimitiveTypeNode(Token id, ValueType valty)
    : identifier(id),
      type(valty) {}
};

struct GenericTypeNode : public TypeNode {
  using Generics = std::vector<pTypeNode>;
  Token identifier;
  Generics generics;
  Modifiers modifiers;

  std::string to_string(uint32_t&) override;
  std::string to_string_x() override;

  pTypeNode clone() override;

  void decay(NodeVisitor&, pTypeNode&) override;

  GenericTypeNode(Token id, Generics gens, Modifiers modifiers)
    : identifier(id),
      generics(std::move(gens)),
      modifiers(modifiers) {}
};

struct UnionTypeNode : public TypeNode {
  pTypeNode lhs;
  pTypeNode rhs;

  std::string to_string(uint32_t&) override;
  std::string to_string_x() override;

  pTypeNode clone() override;

  void decay(NodeVisitor&, pTypeNode&) override;

  UnionTypeNode(pTypeNode lhs, pTypeNode rhs)
    : lhs(std::move(lhs)),
      rhs(std::move(rhs)) {}
};

struct FunctionTypeNode : public TypeNode {
  using parameter_vector = std::vector<pTypeNode>;

  parameter_vector parameters;
  pTypeNode returns;

  std::string to_string(uint32_t&) override;
  std::string to_string_x() override;

  pTypeNode clone() override;

  void decay(NodeVisitor&, pTypeNode&) override;

  FunctionTypeNode(parameter_vector args, pTypeNode rets)
    : parameters(std::move(args)),
      returns(std::move(rets)) {}
};

// =========================================================================================
// Statement Nodes
//
struct DeclarationStmtNode : public StmtNode {
  bool is_global;
  Modifiers modifiers;
  Token identifier;
  pExprNode value_expression;
  pTypeNode type;

  std::string to_string(uint32_t&) override;

  pStmtNode clone() override;

  void accept(NodeVisitor&) override;

  DeclarationStmtNode(
    bool is_global, Modifiers modifiers, Token identifier, pExprNode value, pTypeNode type
  )
    : is_global(is_global),
      modifiers(modifiers),
      identifier(identifier),
      value_expression(std::move(value)),
      type(std::move(type)) {}
};

struct ScopeStmtNode : public StmtNode {
  using Statements = std::vector<pStmtNode>;
  Statements statements;

  std::string to_string(uint32_t&) override;

  pStmtNode clone() override;

  void accept(NodeVisitor&) override;

  ScopeStmtNode(Statements statements)
    : statements(std::move(statements)) {}
};

struct ParameterNode {
  Token identifier;
  Modifiers modifiers;
  pTypeNode type;

  ParameterNode(Token identifier, Modifiers modifiers, pTypeNode type)
    : identifier(identifier),
      modifiers(modifiers),
      type(std::move(type)) {}
};

struct FunctionStmtNode : public StmtNode {
  using Parameters = std::vector<ParameterNode>;

  struct StackNode {
    bool is_global;

    size_t upvalues;

    Modifiers modifiers;
    Token identifier;
    Parameters parameters;

    StackNode(
      bool is_global, size_t upvalues, Modifiers modifiers, Token identifier, Parameters parameters
    )
      : is_global(is_global),
        upvalues(upvalues),
        modifiers(modifiers),
        identifier(identifier),
        parameters(std::move(parameters)) {}
  };

  bool is_global;
  Modifiers modifiers;
  Token identifier;
  pStmtNode body;
  pTypeNode returns;
  Parameters parameters;

  std::string to_string(uint32_t&) override;

  pStmtNode clone() override;

  void accept(NodeVisitor&) override;

  FunctionStmtNode(
    bool is_global,
    Modifiers modifiers,
    Token identifier,
    pStmtNode body,
    pTypeNode returns,
    Parameters parameters
  )
    : is_global(is_global),
      modifiers(modifiers),
      identifier(identifier),
      body(std::move(body)),
      returns(std::move(returns)),
      parameters(std::move(parameters)) {}
};

struct AssignStmtNode : public StmtNode {
  pExprNode assignee;
  Token augmentation_operator;
  pExprNode value;

  std::string to_string(uint32_t&) override;
  pStmtNode clone() override;

  void accept(NodeVisitor&) override;

  AssignStmtNode(pExprNode assignee, Token augment, pExprNode value)
    : assignee(std::move(assignee)),
      augmentation_operator(augment),
      value(std::move(value)) {}
};

struct IfStmtNode : public StmtNode {
  struct ElseIfNode {
    pExprNode condition;
    pStmtNode scope;

    ElseIfNode(pExprNode condition, pStmtNode scope)
      : condition(std::move(condition)),
        scope(std::move(scope)) {}
  };

  using ElseIfNodes = std::vector<ElseIfNode>;

  pExprNode condition;
  pStmtNode scope;
  pStmtNode else_node;
  ElseIfNodes elseif_nodes;

  std::string to_string(uint32_t&) override;
  pStmtNode clone() override;

  void accept(NodeVisitor&) override;

  IfStmtNode(pExprNode condition, pStmtNode scope, pStmtNode else_node, ElseIfNodes elseif_nodes)
    : condition(std::move(condition)),
      scope(std::move(scope)),
      else_node(std::move(else_node)),
      elseif_nodes(std::move(elseif_nodes)) {}
};

struct ReturnStmtNode : public StmtNode {
  pExprNode expression;

  std::string to_string(uint32_t&) override;
  pStmtNode clone() override;

  void accept(NodeVisitor&) override;

  ReturnStmtNode(pExprNode expression)
    : expression(std::move(expression)) {}
};

struct BreakStmtNode : public StmtNode {
  Token token;

  std::string to_string(uint32_t&) override;
  pStmtNode clone() override;

  void accept(NodeVisitor&) override;

  BreakStmtNode(Token tok)
    : token(tok) {}
};

struct ContinueStmtNode : public StmtNode {
  Token token;

  std::string to_string(uint32_t&) override;
  pStmtNode clone() override;

  void accept(NodeVisitor&) override;

  ContinueStmtNode(Token tok)
    : token(tok) {}
};

struct WhileStmtNode : public StmtNode {
  pExprNode condition;
  pStmtNode body;

  std::string to_string(uint32_t&) override;
  pStmtNode clone() override;

  void accept(NodeVisitor&) override;

  WhileStmtNode(pExprNode condition, pStmtNode body)
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

class SyntaxTree {
public:
  std::vector<pStmtNode> statements;
};

VIA_NAMESPACE_END

#endif
