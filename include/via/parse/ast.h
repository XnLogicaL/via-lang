//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_AST_H
#define VIA_HAS_HEADER_AST_H

#include "object.h"
#include "token.h"
#include "ast-base.h"

//  =======
// [ ast.h ]
//  =======
//
// This file declares the derived abstract syntax tree node classes,
// see `ast-base.h` for base class definitions.
//
namespace via {

//  ==================
// [ Expression Nodes ]
//  ==================

struct LitExprNode : public ExprNodeBase {
  using variant = std::variant<std::monostate, int, float, bool, std::string>;

  Token value_token;
  variant value;

  std::string to_string(uint32_t&) override;
  TypeNodeBase* infer_type(TransUnitContext&) override;
  void accept(NodeVisitorBase&, uint32_t) override;

  LitExprNode(Token value_token, variant value)
    : value_token(value_token),
      value(value) {
    this->begin = this->value_token.position;
    this->end = this->value_token.position + value_token.lexeme.length();

    if (value_token.type == TokenType::LIT_STRING) {
      // Shift end position by 2 to account for quotes
      this->end += 2;
    }
  }
};

struct SymExprNode : public ExprNodeBase {
  Token identifier;

  std::string to_string(uint32_t&) override;
  TypeNodeBase* infer_type(TransUnitContext&) override;
  void accept(NodeVisitorBase&, uint32_t) override;

  SymExprNode(Token identifier)
    : identifier(identifier) {
    this->begin = this->identifier.position;
    this->end = this->identifier.position + identifier.lexeme.length();
  }
};

struct UnaryExprNode : public ExprNodeBase {
  ExprNodeBase* expression;

  std::string to_string(uint32_t&) override;
  TypeNodeBase* infer_type(TransUnitContext&) override;

  void accept(NodeVisitorBase&, uint32_t) override;

  UnaryExprNode(ExprNodeBase* expression)
    : expression(std::move(expression)) {
    this->begin = this->expression->begin - 1; // Account for '-'
    this->end = this->expression->end;
  }
};

struct GroupExprNode : public ExprNodeBase {
  ExprNodeBase* expression;

  std::string to_string(uint32_t&) override;
  TypeNodeBase* infer_type(TransUnitContext&) override;
  void accept(NodeVisitorBase&, uint32_t) override;
  int precedence() const override;

  GroupExprNode(ExprNodeBase* expression)
    : expression(std::move(expression)) {
    this->begin = this->expression->begin - 1; // Account for '('
    this->end = this->expression->end + 1;     // Account for ')'
  }
};

struct CallExprNode : public ExprNodeBase {
  using argument_vector = std::vector<ExprNodeBase*>;

  ExprNodeBase* callee;
  argument_vector arguments;

  std::string to_string(uint32_t&) override;
  TypeNodeBase* infer_type(TransUnitContext&) override;
  void accept(NodeVisitorBase&, uint32_t) override;

  CallExprNode(ExprNodeBase* callee, argument_vector arguments)
    : callee(std::move(callee)),
      arguments(std::move(arguments)) {

    if (!this->arguments.empty()) {
      ExprNodeBase*& last_arg = this->arguments.back();

      this->begin = this->callee->begin;
      this->end = last_arg->end + 1; // Account for ')'
    }
    else {
      this->begin = this->callee->begin;
      this->end = this->callee->end + 2; // Account for '()'
    }
  }
};

struct IndexExprNode : public ExprNodeBase {
  ExprNodeBase* object;
  ExprNodeBase* index;

  std::string to_string(uint32_t&) override;
  TypeNodeBase* infer_type(TransUnitContext&) override;
  void accept(NodeVisitorBase&, uint32_t) override;

  IndexExprNode(ExprNodeBase* object, ExprNodeBase* index)
    : object(std::move(object)),
      index(std::move(index)) {
    this->begin = this->object->begin;
    this->end = this->index->end;
  }
};

struct BinExprNode : public ExprNodeBase {
  Token op;
  ExprNodeBase* lhs_expression;
  ExprNodeBase* rhs_expression;

  std::string to_string(uint32_t&) override;
  TypeNodeBase* infer_type(TransUnitContext&) override;
  void accept(NodeVisitorBase&, uint32_t) override;

  BinExprNode(Token op, ExprNodeBase* lhs, ExprNodeBase* rhs)
    : op(op),
      lhs_expression(std::move(lhs)),
      rhs_expression(std::move(rhs)) {
    this->begin = this->lhs_expression->begin;
    this->end = this->rhs_expression->end;
  }
};

struct CastExprNode : public ExprNodeBase {
  ExprNodeBase* expression;
  TypeNodeBase* type;

  std::string to_string(uint32_t&) override;
  TypeNodeBase* infer_type(TransUnitContext&) override;
  void accept(NodeVisitorBase&, uint32_t) override;

  CastExprNode(ExprNodeBase* expression, TypeNodeBase* type)
    : expression(std::move(expression)),
      type(std::move(type)) {
    this->begin = this->expression->begin;
    this->end = this->expression->end;
  }
};

struct StepExprNode : public ExprNodeBase {
  ExprNodeBase* target;
  bool is_increment;
  bool is_postfix;

  std::string to_string(uint32_t&) override;
  TypeNodeBase* infer_type(TransUnitContext&) override;

  void accept(NodeVisitorBase&, uint32_t) override;

  StepExprNode(ExprNodeBase* target, bool is_increment, bool is_postfix)
    : target(std::move(target)),
      is_increment(is_increment),
      is_postfix(is_postfix) {
    if (is_postfix) {
      this->begin = this->target->begin;
      this->end = this->target->end + 2;
    }
    else {
      this->begin = this->target->begin - 2;
      this->end = this->target->end;
    }
  }
};

struct ArrayExprNode : public ExprNodeBase {
  using values_t = std::vector<ExprNodeBase*>;

  Token open_brace;
  Token close_brace;
  values_t values;

  std::string to_string(uint32_t&) override;
  TypeNodeBase* infer_type(TransUnitContext&) override;

  void accept(NodeVisitorBase&, uint32_t) override;

  ArrayExprNode(Token open_brace, Token close_brace, values_t values)
    : open_brace(open_brace),
      close_brace(close_brace),
      values(std::move(values)) {}
};

// =========================================================================================
// Type Nodes
//
struct AutoTypeNode : public TypeNodeBase {
  std::string to_string(uint32_t&) override;
  std::string to_output_string() override;
  void decay(NodeVisitorBase&, TypeNodeBase*&) override;

  AutoTypeNode(size_t begin, size_t end) {
    this->begin = begin;
    this->end = end;
  }
};

struct PrimTypeNode : public TypeNodeBase {
  Token identifier;
  IValueType type;

  std::string to_string(uint32_t&) override;
  std::string to_output_string() override;

  PrimTypeNode(Token id, IValueType valty)
    : identifier(id),
      type(valty) {
    this->begin = id.position;
    this->end = id.position + id.lexeme.length();
  }
};

struct GenericTypeNode : public TypeNodeBase {
  using generics_t = std::vector<TypeNodeBase*>;

  Token identifier;
  generics_t generics;
  StmtModifiers modifs;

  std::string to_string(uint32_t&) override;
  std::string to_output_string() override;
  void decay(NodeVisitorBase&, TypeNodeBase*&) override;

  GenericTypeNode(Token id, generics_t gens, StmtModifiers modifs)
    : identifier(id),
      generics(std::move(gens)),
      modifs(modifs) {
    this->begin = id.position;
    this->end = id.position + id.lexeme.length();
  }
};

struct UnionTypeNode : public TypeNodeBase {
  TypeNodeBase* lhs;
  TypeNodeBase* rhs;

  std::string to_string(uint32_t&) override;
  std::string to_output_string() override;
  void decay(NodeVisitorBase&, TypeNodeBase*&) override;

  UnionTypeNode(TypeNodeBase* lhs, TypeNodeBase* rhs)
    : lhs(std::move(lhs)),
      rhs(std::move(rhs)) {
    this->begin = this->lhs->begin;
    this->end = this->rhs->end;
  }
};

struct ParamNode {
  Token identifier;
  StmtModifiers modifs;
  TypeNodeBase* type;

  ParamNode(Token identifier, StmtModifiers modifs, TypeNodeBase* type)
    : identifier(identifier),
      modifs(modifs),
      type(std::move(type)) {}
};

struct FunctionTypeNode : public TypeNodeBase {
  using parameter_vector = std::vector<ParamNode>;

  parameter_vector parameters;
  TypeNodeBase* returns;

  std::string to_string(uint32_t&) override;
  std::string to_output_string() override;
  void decay(NodeVisitorBase&, TypeNodeBase*&) override;

  FunctionTypeNode(parameter_vector args, TypeNodeBase* rets)
    : parameters(std::move(args)),
      returns(std::move(rets)) {
    this->begin = this->returns->begin;
    this->end = this->returns->end;
  }
};

// =========================================================================================
// Statement Nodes
//
struct DeclStmtNode : public StmtNodeBase {
  bool is_global;
  StmtModifiers modifs;
  Token identifier;
  ExprNodeBase* value_expression;
  TypeNodeBase* type;

  std::string to_string(uint32_t&) override;
  void accept(NodeVisitorBase&) override;

  DeclStmtNode(
    bool is_global, StmtModifiers modifs, Token identifier, ExprNodeBase* value, TypeNodeBase* type
  )
    : is_global(is_global),
      modifs(modifs),
      identifier(identifier),
      value_expression(std::move(value)),
      type(std::move(type)) {}
};

struct ScopeStmtNode : public StmtNodeBase {
  using Statements = std::vector<StmtNodeBase*>;
  Statements statements;

  std::string to_string(uint32_t&) override;
  void accept(NodeVisitorBase&) override;

  ScopeStmtNode(Statements statements)
    : statements(std::move(statements)) {}
};

struct FuncDeclStmtNode : public StmtNodeBase {
  using parameters_t = std::vector<ParamNode>;

  bool is_global;
  StmtModifiers modifs;
  Token identifier;
  StmtNodeBase* body;
  TypeNodeBase* returns;
  parameters_t parameters;

  std::string to_string(uint32_t&) override;
  void accept(NodeVisitorBase&) override;

  FuncDeclStmtNode(
    bool is_global,
    StmtModifiers modifs,
    Token identifier,
    StmtNodeBase* body,
    TypeNodeBase* returns,
    parameters_t parameters
  )
    : is_global(is_global),
      modifs(modifs),
      identifier(identifier),
      body(std::move(body)),
      returns(std::move(returns)),
      parameters(std::move(parameters)) {}
};

struct AssignStmtNode : public StmtNodeBase {
  ExprNodeBase* assignee;
  Token augmentation_operator;
  ExprNodeBase* value;

  std::string to_string(uint32_t&) override;
  void accept(NodeVisitorBase&) override;

  AssignStmtNode(ExprNodeBase* assignee, Token augment, ExprNodeBase* value)
    : assignee(std::move(assignee)),
      augmentation_operator(augment),
      value(std::move(value)) {}
};

struct IfStmtNode : public StmtNodeBase {
  struct elseif_node {
    ExprNodeBase* condition;
    StmtNodeBase* scope;

    elseif_node(ExprNodeBase* condition, StmtNodeBase* scope)
      : condition(std::move(condition)),
        scope(std::move(scope)) {}
  };

  using elseif_nodes_t = std::vector<elseif_node>;

  ExprNodeBase* condition;
  StmtNodeBase* scope;
  StmtNodeBase* else_node;
  elseif_nodes_t elseif_nodes;

  std::string to_string(uint32_t&) override;
  void accept(NodeVisitorBase&) override;

  IfStmtNode(
    ExprNodeBase* condition,
    StmtNodeBase* scope,
    StmtNodeBase* else_node,
    elseif_nodes_t elseif_nodes
  )
    : condition(std::move(condition)),
      scope(std::move(scope)),
      else_node(std::move(else_node)),
      elseif_nodes(std::move(elseif_nodes)) {}
};

struct ReturnStmtNode : public StmtNodeBase {
  ExprNodeBase* expression;

  std::string to_string(uint32_t&) override;
  void accept(NodeVisitorBase&) override;

  ReturnStmtNode(ExprNodeBase* expression)
    : expression(std::move(expression)) {}
};

struct BreakStmtNode : public StmtNodeBase {
  Token tok;

  std::string to_string(uint32_t&) override;
  void accept(NodeVisitorBase&) override;

  BreakStmtNode(Token tok)
    : tok(tok) {}
};

struct ContinueStmtNode : public StmtNodeBase {
  Token tok;

  std::string to_string(uint32_t&) override;
  void accept(NodeVisitorBase&) override;

  ContinueStmtNode(Token tok)
    : tok(tok) {}
};

struct WhileStmtNode : public StmtNodeBase {
  ExprNodeBase* condition;
  StmtNodeBase* body;

  std::string to_string(uint32_t&) override;
  void accept(NodeVisitorBase&) override;

  WhileStmtNode(ExprNodeBase* condition, StmtNodeBase* body)
    : condition(std::move(condition)),
      body(std::move(body)) {}
};

struct ExprStmtNode : public StmtNodeBase {
  ExprNodeBase* expression;

  std::string to_string(uint32_t&) override;
  void accept(NodeVisitorBase&) override;

  ExprStmtNode(ExprNodeBase* expression)
    : expression(std::move(expression)) {}
};

class SyntaxTree {
public:
  VIA_IMPLEMENTATION SyntaxTree()
    : allocator(64 * 1024 * 1024) {}

  ArenaAllocator allocator;
  std::vector<StmtNodeBase*> statements;
};

} // namespace via

#endif
