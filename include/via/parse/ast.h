// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_ast_h
#define vl_has_header_ast_h

#include "object.h"
#include "token.h"
#include "ast-base.h"

// ===========================================================================================
// ast.h
//
// This file declares the derived abstract syntax tree node classes,
// see `ast-base.h` for base class definitions.
//
namespace via {

// =========================================================================================
// Expression Nodes
//
struct lit_expr_node : public expr_node_base {
  using variant = std::variant<std::monostate, int, float, bool, std::string>;

  token value_token;
  variant value;

  std::string to_string(uint32_t&) override;

  p_expr_node_t clone() override;
  p_type_node_t infer_type(trans_unit_context&) override;

  void accept(node_visitor_base&, uint32_t) override;

  lit_expr_node(token value_token, variant value)
    : value_token(value_token),
      value(value) {
    this->begin = this->value_token.position;
    this->end = this->value_token.position + value_token.lexeme.length();
  }
};

struct sym_expr_node : public expr_node_base {
  token identifier;

  std::string to_string(uint32_t&) override;

  p_expr_node_t clone() override;
  p_type_node_t infer_type(trans_unit_context&) override;

  void accept(node_visitor_base&, uint32_t) override;

  sym_expr_node(token identifier)
    : identifier(identifier) {
    this->begin = this->identifier.position;
    this->end = this->identifier.position + identifier.lexeme.length();
  }
};

struct unary_expr_node : public expr_node_base {
  p_expr_node_t expression;

  std::string to_string(uint32_t&) override;

  p_expr_node_t clone() override;
  p_type_node_t infer_type(trans_unit_context&) override;

  void accept(node_visitor_base&, uint32_t) override;

  unary_expr_node(p_expr_node_t expression)
    : expression(std::move(expression)) {
    this->begin = this->expression->begin - 1; // Account for '-'
    this->end = this->expression->end;
  }
};

struct grp_expr_node : public expr_node_base {
  p_expr_node_t expression;

  std::string to_string(uint32_t&) override;

  p_expr_node_t clone() override;
  p_type_node_t infer_type(trans_unit_context&) override;

  void accept(node_visitor_base&, uint32_t) override;
  int precedence() const override;

  grp_expr_node(p_expr_node_t expression)
    : expression(std::move(expression)) {
    this->begin = this->expression->begin - 1; // Account for '('
    this->end = this->expression->end + 1;     // Account for ')'
  }
};

struct call_expr_node : public expr_node_base {
  using argument_vector = std::vector<p_expr_node_t>;

  p_expr_node_t callee;
  argument_vector arguments;

  std::string to_string(uint32_t&) override;

  p_expr_node_t clone() override;
  p_type_node_t infer_type(trans_unit_context&) override;

  void accept(node_visitor_base&, uint32_t) override;

  call_expr_node(p_expr_node_t callee, argument_vector arguments)
    : callee(std::move(callee)),
      arguments(std::move(arguments)) {

    if (!this->arguments.empty()) {
      p_expr_node_t& last_arg = this->arguments.back();

      this->begin = this->callee->begin;
      this->end = last_arg->end + 1; // Account for ')'
    }
    else {
      this->begin = this->callee->begin;
      this->end = this->callee->end + 2; // Account for '()'
    }
  }
};

struct index_expr_node : public expr_node_base {
  p_expr_node_t object;
  p_expr_node_t index;

  std::string to_string(uint32_t&) override;

  p_expr_node_t clone() override;
  p_type_node_t infer_type(trans_unit_context&) override;

  void accept(node_visitor_base&, uint32_t) override;

  index_expr_node(p_expr_node_t object, p_expr_node_t index)
    : object(std::move(object)),
      index(std::move(index)) {
    this->begin = this->object->begin;
    this->end = this->index->end;
  }
};

struct bin_expr_node : public expr_node_base {
  token op;
  p_expr_node_t lhs_expression;
  p_expr_node_t rhs_expression;

  std::string to_string(uint32_t&) override;

  p_expr_node_t clone() override;
  p_type_node_t infer_type(trans_unit_context&) override;

  void accept(node_visitor_base&, uint32_t) override;

  bin_expr_node(token op, p_expr_node_t lhs, p_expr_node_t rhs)
    : op(op),
      lhs_expression(std::move(lhs)),
      rhs_expression(std::move(rhs)) {
    this->begin = this->lhs_expression->begin;
    this->end = this->rhs_expression->end;
  }
};

struct cast_expr_node : public expr_node_base {
  p_expr_node_t expression;
  p_type_node_t type;

  std::string to_string(uint32_t&) override;

  p_expr_node_t clone() override;
  p_type_node_t infer_type(trans_unit_context&) override;

  void accept(node_visitor_base&, uint32_t) override;

  cast_expr_node(p_expr_node_t expression, p_type_node_t type)
    : expression(std::move(expression)),
      type(std::move(type)) {
    this->begin = this->expression->begin;
    this->end = this->expression->end;
  }
};

struct table_expr_node : public expr_node_base {
  struct KvPair {
    p_expr_node_t key;
    p_expr_node_t val;
  };

  using kvpair_vector = std::vector<KvPair>;

  token open_brace;
  token close_brace;
  kvpair_vector pairs;

  std::string to_string(uint32_t&) override;

  p_expr_node_t clone() override;
  p_type_node_t infer_type(trans_unit_context&) override;

  void accept(node_visitor_base&, uint32_t) override;

  table_expr_node(token open_brace, token close_brace, kvpair_vector pairs)
    : pairs(std::move(pairs)) {
    this->begin = open_brace.position;
    this->end = close_brace.position;
  }
};

// =========================================================================================
// Type Nodes
//
struct auto_type_node : public type_node_base {
  std::string to_string(uint32_t&) override;
  std::string to_string_x() override;

  p_type_node_t clone() override;

  void decay(node_visitor_base&, p_type_node_t&) override;
};

struct primitive_type_node : public type_node_base {
  token identifier;
  value_type type;

  std::string to_string(uint32_t&) override;
  std::string to_string_x() override;

  p_type_node_t clone() override;

  primitive_type_node(token id, value_type valty)
    : identifier(id),
      type(valty) {}
};

struct generic_type_node : public type_node_base {
  using Generics = std::vector<p_type_node_t>;
  token identifier;
  Generics generics;
  modifiers modifs;

  std::string to_string(uint32_t&) override;
  std::string to_string_x() override;

  p_type_node_t clone() override;

  void decay(node_visitor_base&, p_type_node_t&) override;

  generic_type_node(token id, Generics gens, modifiers modifs)
    : identifier(id),
      generics(std::move(gens)),
      modifs(modifs) {}
};

struct union_type_node : public type_node_base {
  p_type_node_t lhs;
  p_type_node_t rhs;

  std::string to_string(uint32_t&) override;
  std::string to_string_x() override;

  p_type_node_t clone() override;

  void decay(node_visitor_base&, p_type_node_t&) override;

  union_type_node(p_type_node_t lhs, p_type_node_t rhs)
    : lhs(std::move(lhs)),
      rhs(std::move(rhs)) {}
};

struct FunctionTypeNode : public type_node_base {
  using parameter_vector = std::vector<p_type_node_t>;

  parameter_vector parameters;
  p_type_node_t returns;

  std::string to_string(uint32_t&) override;
  std::string to_string_x() override;

  p_type_node_t clone() override;

  void decay(node_visitor_base&, p_type_node_t&) override;

  FunctionTypeNode(parameter_vector args, p_type_node_t rets)
    : parameters(std::move(args)),
      returns(std::move(rets)) {}
};

// =========================================================================================
// Statement Nodes
//
struct decl_stmt_node : public stmt_node_base {
  bool is_global;
  modifiers modifs;
  token identifier;
  p_expr_node_t value_expression;
  p_type_node_t type;

  std::string to_string(uint32_t&) override;

  p_stmt_node_t clone() override;

  void accept(node_visitor_base&) override;

  decl_stmt_node(
    bool is_global, modifiers modifs, token identifier, p_expr_node_t value, p_type_node_t type
  )
    : is_global(is_global),
      modifs(modifs),
      identifier(identifier),
      value_expression(std::move(value)),
      type(std::move(type)) {}
};

struct scope_stmt_node : public stmt_node_base {
  using Statements = std::vector<p_stmt_node_t>;
  Statements statements;

  std::string to_string(uint32_t&) override;

  p_stmt_node_t clone() override;

  void accept(node_visitor_base&) override;

  scope_stmt_node(Statements statements)
    : statements(std::move(statements)) {}
};

struct param_node {
  token identifier;
  modifiers modifs;
  p_type_node_t type;

  param_node(token identifier, modifiers modifs, p_type_node_t type)
    : identifier(identifier),
      modifs(modifs),
      type(std::move(type)) {}
};

struct func_stmt_node : public stmt_node_base {
  using parameters_t = std::vector<param_node>;

  struct stack_node {
    bool is_global;

    size_t upvalues;

    modifiers modifs;
    token identifier;
    parameters_t parameters;

    stack_node(
      bool is_global, size_t upvalues, modifiers modifs, token identifier, parameters_t parameters
    )
      : is_global(is_global),
        upvalues(upvalues),
        modifs(modifs),
        identifier(identifier),
        parameters(std::move(parameters)) {}
  };

  bool is_global;
  modifiers modifs;
  token identifier;
  p_stmt_node_t body;
  p_type_node_t returns;
  parameters_t parameters;

  std::string to_string(uint32_t&) override;

  p_stmt_node_t clone() override;

  void accept(node_visitor_base&) override;

  func_stmt_node(
    bool is_global,
    modifiers modifs,
    token identifier,
    p_stmt_node_t body,
    p_type_node_t returns,
    parameters_t parameters
  )
    : is_global(is_global),
      modifs(modifs),
      identifier(identifier),
      body(std::move(body)),
      returns(std::move(returns)),
      parameters(std::move(parameters)) {}
};

struct assign_stmt_node : public stmt_node_base {
  p_expr_node_t assignee;
  token augmentation_operator;
  p_expr_node_t value;

  std::string to_string(uint32_t&) override;
  p_stmt_node_t clone() override;

  void accept(node_visitor_base&) override;

  assign_stmt_node(p_expr_node_t assignee, token augment, p_expr_node_t value)
    : assignee(std::move(assignee)),
      augmentation_operator(augment),
      value(std::move(value)) {}
};

struct if_stmt_node : public stmt_node_base {
  struct elseif_node {
    p_expr_node_t condition;
    p_stmt_node_t scope;

    elseif_node(p_expr_node_t condition, p_stmt_node_t scope)
      : condition(std::move(condition)),
        scope(std::move(scope)) {}
  };

  using elseif_nodes_t = std::vector<elseif_node>;

  p_expr_node_t condition;
  p_stmt_node_t scope;
  p_stmt_node_t else_node;
  elseif_nodes_t elseif_nodes;

  std::string to_string(uint32_t&) override;
  p_stmt_node_t clone() override;

  void accept(node_visitor_base&) override;

  if_stmt_node(
    p_expr_node_t condition,
    p_stmt_node_t scope,
    p_stmt_node_t else_node,
    elseif_nodes_t elseif_nodes
  )
    : condition(std::move(condition)),
      scope(std::move(scope)),
      else_node(std::move(else_node)),
      elseif_nodes(std::move(elseif_nodes)) {}
};

struct return_stmt_node : public stmt_node_base {
  p_expr_node_t expression;

  std::string to_string(uint32_t&) override;
  p_stmt_node_t clone() override;

  void accept(node_visitor_base&) override;

  return_stmt_node(p_expr_node_t expression)
    : expression(std::move(expression)) {}
};

struct break_stmt_node : public stmt_node_base {
  token tok;

  std::string to_string(uint32_t&) override;
  p_stmt_node_t clone() override;

  void accept(node_visitor_base&) override;

  break_stmt_node(token tok)
    : tok(tok) {}
};

struct continue_stmt_node : public stmt_node_base {
  token tok;

  std::string to_string(uint32_t&) override;
  p_stmt_node_t clone() override;

  void accept(node_visitor_base&) override;

  continue_stmt_node(token tok)
    : tok(tok) {}
};

struct while_stmt_node : public stmt_node_base {
  p_expr_node_t condition;
  p_stmt_node_t body;

  std::string to_string(uint32_t&) override;
  p_stmt_node_t clone() override;

  void accept(node_visitor_base&) override;

  while_stmt_node(p_expr_node_t condition, p_stmt_node_t body)
    : condition(std::move(condition)),
      body(std::move(body)) {}
};

struct expr_stmt_node : public stmt_node_base {
  p_expr_node_t expression;

  std::string to_string(uint32_t&) override;

  p_stmt_node_t clone() override;

  void accept(node_visitor_base&) override;

  expr_stmt_node(p_expr_node_t expression)
    : expression(std::move(expression)) {}
};

class syntax_tree {
public:
  std::vector<p_stmt_node_t> statements;
};

} // namespace via

#endif
