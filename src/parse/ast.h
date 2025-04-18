// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_AST_H
#define VIA_HAS_HEADER_AST_H

#include "tvalue.h"
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

#define VIA_DECLASTFUNCS()                                                                         \
  std::string to_string(uint32_t&) override;                                                       \
  TypeNodeBase* infer_type(TransUnitContext&) override;                                            \
  void accept(NodeVisitorBase&, uint32_t) override;

/**
 * Literal Expression Node
 * Represents a primitive literal value. Can be a Nil, Int, floating point, Bool or String
 * value.
 *
 * <literal> ::= <Int> | <float> | <String> | <Bool> | "Nil"
 * <Int> ::= ("0x"|"0b")?[0-9A-Fa-f]+
 * <float>   ::= [0-9]+.[0-9]+
 * <String>  ::= "\"" <characters> "\""
 * <Bool> ::= "true" | "false"
 */
struct LitExprNode : public ExprNodeBase {
  using variant = std::variant<std::monostate, int, float, bool, std::string>;

  Token value_token;
  variant value;

  VIA_DECLASTFUNCS();

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

/**
 * Symbol Expression Node
 * Represents a symbol that could be a variable, index, property or global.
 *
 * <identifier>  ::= [A-Za-z_][A-Za-z0-9_]+
 * <symbol_expr> ::= <identifier>
 */
struct SymExprNode : public ExprNodeBase {
  Token identifier;

  VIA_DECLASTFUNCS();

  SymExprNode(Token identifier)
    : identifier(identifier) {
    this->begin = this->identifier.position;
    this->end = this->identifier.position + identifier.lexeme.length();
  }
};

/**
 * Unary Expression Node
 * Represents a unary expression node that applies an operator on a single expression.
 *
 * <operator>   ::= "+" | "-" | "*" | "/" | "++" | "--" | "#" | "^" | "%"
 * <unary_expr> ::= <operator> <expression>
 */
struct UnaryExprNode : public ExprNodeBase {
  Token op;
  ExprNodeBase* expression;

  VIA_DECLASTFUNCS();

  UnaryExprNode(Token op, ExprNodeBase* expression)
    : op(op),
      expression(expression) {
    this->begin = this->expression->begin - 1; // Account for '-'
    this->end = this->expression->end;
  }
};

/**
 * Group Expression Node
 * Represents a grouping expression that houses a single expression. Mainly used on the parser level
 * to determine operator precedence.
 *
 * <group_expr> ::= "(" <expression> ")"
 */
struct GroupExprNode : public ExprNodeBase {
  ExprNodeBase* expression;

  VIA_DECLASTFUNCS();
  int precedence() const override;

  GroupExprNode(ExprNodeBase* expression)
    : expression(expression) {
    this->begin = this->expression->begin - 1; // Account for '('
    this->end = this->expression->end + 1;     // Account for ')'
  }
};

/**
 * Call Expression Node
 * Represents a function call expression with an argument list.
 *
 * <call_expr> ::= <expression> "(" <arg_list>? ")"
 * <arg_list>  ::= <expression> ("," <expression>)*
 */
struct CallExprNode : public ExprNodeBase {
  using argument_vector = std::vector<ExprNodeBase*>;

  ExprNodeBase* callee;
  argument_vector arguments;

  VIA_DECLASTFUNCS();

  CallExprNode(ExprNodeBase* callee, argument_vector arguments)
    : callee(callee),
      arguments(arguments) {

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

/**
 * Index Expression Node
 * Represents a subscript expression that holds a target and index expression.
 *
 * <index_expr>   ::= <primary_expr> <accessor>*
 * <accessor>     ::= "[" <expression> "]" | "." <identifier>
 * <primary_expr> ::= <identifier> | <literal> | "(" <expression> ")"
 */
struct IndexExprNode : public ExprNodeBase {
  ExprNodeBase* object;
  ExprNodeBase* index;

  VIA_DECLASTFUNCS();

  IndexExprNode(ExprNodeBase* object, ExprNodeBase* index)
    : object(object),
      index(index) {
    this->begin = this->object->begin;
    this->end = this->index->end;
  }
};

struct BinExprNode : public ExprNodeBase {
  Token op;
  ExprNodeBase* lhs_expression;
  ExprNodeBase* rhs_expression;

  VIA_DECLASTFUNCS();

  BinExprNode(Token op, ExprNodeBase* lhs, ExprNodeBase* rhs)
    : op(op),
      lhs_expression(lhs),
      rhs_expression(rhs) {
    this->begin = this->lhs_expression->begin;
    this->end = this->rhs_expression->end;
  }
};

struct CastExprNode : public ExprNodeBase {
  ExprNodeBase* expression;
  TypeNodeBase* type;

  VIA_DECLASTFUNCS();

  CastExprNode(ExprNodeBase* expression, TypeNodeBase* type)
    : expression(expression),
      type(type) {
    this->begin = this->expression->begin;
    this->end = this->expression->end;
  }
};

struct StepExprNode : public ExprNodeBase {
  bool is_increment;
  ExprNodeBase* target;

  VIA_DECLASTFUNCS();

  StepExprNode(ExprNodeBase* target, bool is_increment)
    : is_increment(is_increment),
      target(target) {
    this->begin = this->target->begin;
    this->end = this->target->end + 2;
  }
};

struct ArrayExprNode : public ExprNodeBase {
  using values_t = std::vector<ExprNodeBase*>;
  values_t values;

  VIA_DECLASTFUNCS();

  ArrayExprNode(size_t begin, size_t end, values_t values)
    : values(values) {
    this->begin = begin;
    this->end = end;
  }
};

// =========================================================================================
// Type Nodes
//

#undef VIA_DECLASTFUNCS
#define VIA_DECLDECAY() void decay(NodeVisitorBase&, TypeNodeBase*&) override;
#define VIA_DECLASTFUNCS()                                                                         \
  std::string to_string(uint32_t&) override;                                                       \
  std::string to_output_string() override;

struct AutoTypeNode : public TypeNodeBase {
  VIA_DECLASTFUNCS();
  VIA_DECLDECAY();

  AutoTypeNode(size_t begin, size_t end) {
    this->begin = begin;
    this->end = end;
  }
};

struct PrimTypeNode : public TypeNodeBase {
  Token identifier;
  Value::Tag type;

  VIA_DECLASTFUNCS();

  PrimTypeNode(Token id, Value::Tag valty)
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

  VIA_DECLASTFUNCS();
  VIA_DECLDECAY();

  GenericTypeNode(Token id, generics_t gens, StmtModifiers modifs)
    : identifier(id),
      generics(gens),
      modifs(modifs) {
    this->begin = id.position;
    this->end = id.position + id.lexeme.length();
  }
};

struct UnionTypeNode : public TypeNodeBase {
  TypeNodeBase* lhs;
  TypeNodeBase* rhs;

  VIA_DECLASTFUNCS();
  VIA_DECLDECAY();

  UnionTypeNode(TypeNodeBase* lhs, TypeNodeBase* rhs)
    : lhs(lhs),
      rhs(rhs) {
    this->begin = this->lhs->begin;
    this->end = this->rhs->end;
  }
};

struct ParamStmtNode : public StmtNodeBase {
  Token identifier;
  StmtModifiers modifs;
  TypeNodeBase* type;

  std::string to_string(uint32_t& depth) override;
  void accept(NodeVisitorBase& visitor) override;

  ParamStmtNode(Token identifier, StmtModifiers modifs, TypeNodeBase* type)
    : identifier(identifier),
      modifs(modifs),
      type(type) {}
};

struct FunctionTypeNode : public TypeNodeBase {
  using parameter_vector = std::vector<ParamStmtNode>;

  parameter_vector parameters;
  TypeNodeBase* returns;

  VIA_DECLASTFUNCS();
  VIA_DECLDECAY();

  FunctionTypeNode(parameter_vector args, TypeNodeBase* rets)
    : parameters(std::move(args)),
      returns(rets) {
    this->begin = this->returns->begin;
    this->end = this->returns->end;
  }
};

struct ArrayTypeNode : public TypeNodeBase {
  TypeNodeBase* type;

  VIA_DECLASTFUNCS();
  VIA_DECLDECAY();

  ArrayTypeNode(TypeNodeBase* type)
    : type(type) {
    this->begin = type->begin - 1;
    this->end = type->end + 1;
  }
};

// =========================================================================================
// Statement Nodes
//

#undef VIA_DECLASTFUNCS
#undef VIA_DECLDECAY
#define VIA_DECLASTFUNCS()                                                                         \
  std::string to_string(uint32_t&) override;                                                       \
  void accept(NodeVisitorBase&) override;

struct DeclStmtNode : public StmtNodeBase {
  bool is_global;
  Token identifier;
  StmtModifiers modifs;
  ExprNodeBase* rvalue;
  TypeNodeBase* type;

  VIA_DECLASTFUNCS();

  DeclStmtNode(
    size_t begin,
    size_t end,
    bool is_global,
    StmtModifiers modifiers,
    Token identifier,
    ExprNodeBase* rvalue,
    TypeNodeBase* type
  )
    : is_global(is_global),
      identifier(identifier),
      modifs(modifiers),
      rvalue(rvalue),
      type(type) {
    this->begin = begin;
    this->end = end;
  }
};

struct ScopeStmtNode : public StmtNodeBase {
  using Statements = std::vector<StmtNodeBase*>;
  Statements statements;

  VIA_DECLASTFUNCS();

  ScopeStmtNode(size_t begin, size_t end, Statements statements)
    : statements(statements) {
    this->begin = begin;
    this->end = end;
  }
};

struct FuncDeclStmtNode : public StmtNodeBase {
  using parameters_t = std::vector<ParamStmtNode>;

  bool is_global;
  StmtModifiers modifs;
  Token identifier;
  StmtNodeBase* body;
  TypeNodeBase* returns;
  parameters_t parameters;

  VIA_DECLASTFUNCS();

  FuncDeclStmtNode(
    size_t begin,
    size_t end,
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
      body(body),
      returns(returns),
      parameters(std::move(parameters)) {
    this->begin = begin;
    this->end = end;
  }
};

struct AssignStmtNode : public StmtNodeBase {
  Token augmentation_operator;
  ExprNodeBase* lvalue;
  ExprNodeBase* rvalue;

  VIA_DECLASTFUNCS();

  AssignStmtNode(ExprNodeBase* lvalue, Token augmentation_operator, ExprNodeBase* rvalue)
    : augmentation_operator(augmentation_operator),
      lvalue(lvalue),
      rvalue(rvalue) {
    this->begin = lvalue->begin;
    this->end = rvalue->end;
  }
};

struct ElseIfNode {
  size_t begin;
  size_t end;

  ExprNodeBase* condition;
  StmtNodeBase* scope;

  ElseIfNode(size_t begin, size_t end, ExprNodeBase* condition, StmtNodeBase* scope)
    : condition(condition),
      scope(scope) {
    this->begin = begin;
    this->end = end;
  }
};

struct IfStmtNode : public StmtNodeBase {
  using elseif_nodes_t = std::vector<ElseIfNode*>;

  ExprNodeBase* condition;
  StmtNodeBase* scope;
  StmtNodeBase* else_node;
  elseif_nodes_t elseif_nodes;

  VIA_DECLASTFUNCS();

  IfStmtNode(
    size_t begin,
    size_t end,
    ExprNodeBase* condition,
    StmtNodeBase* scope,
    StmtNodeBase* else_node,
    elseif_nodes_t elseif_nodes
  )
    : condition(condition),
      scope(scope),
      else_node(else_node),
      elseif_nodes(std::move(elseif_nodes)) {
    this->begin = begin;
    this->end = end;
  }
};

struct ReturnStmtNode : public StmtNodeBase {
  ExprNodeBase* expression;

  VIA_DECLASTFUNCS();

  ReturnStmtNode(size_t begin, size_t end, ExprNodeBase* expression)
    : expression(expression) {
    this->begin = begin;
    this->end = end;
  }
};

struct BreakStmtNode : public StmtNodeBase {
  VIA_DECLASTFUNCS();

  BreakStmtNode(size_t begin, size_t end) {
    this->begin = begin;
    this->end = end;
  }
};

struct ContinueStmtNode : public StmtNodeBase {
  VIA_DECLASTFUNCS();

  ContinueStmtNode(size_t begin, size_t end) {
    this->begin = begin;
    this->end = end;
  }
};

struct WhileStmtNode : public StmtNodeBase {
  ExprNodeBase* condition;
  StmtNodeBase* body;

  VIA_DECLASTFUNCS();

  WhileStmtNode(size_t begin, size_t end, ExprNodeBase* condition, StmtNodeBase* body)
    : condition(condition),
      body(body) {
    this->begin = begin;
    this->end = end;
  }
};

struct DeferStmtNode : public StmtNodeBase {
  StmtNodeBase* stmt;

  VIA_DECLASTFUNCS();

  DeferStmtNode(size_t begin, size_t end, StmtNodeBase* stmt)
    : stmt(stmt) {
    this->begin = begin;
    this->end = end;
  }
};

struct ExprStmtNode : public StmtNodeBase {
  ExprNodeBase* expression;

  VIA_DECLASTFUNCS();

  ExprStmtNode(ExprNodeBase* expression)
    : expression(expression) {
    this->begin = expression->begin;
    this->end = expression->end;
  }
};

class SyntaxTree {
public:
  inline SyntaxTree()
    : allocator(64 * 1024 * 1024) {}

  ArenaAllocator allocator;
  std::vector<StmtNodeBase*> statements;
};

} // namespace via

#endif
