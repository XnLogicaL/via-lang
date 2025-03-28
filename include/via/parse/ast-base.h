// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_ast_base_h
#define vl_has_header_ast_base_h

#include "token.h"

namespace via {

class trans_unit_context;

class node_visitor_base;
struct expr_node_base;
struct stmt_node_base;
struct type_node_base;

using p_expr_node_t = std::unique_ptr<expr_node_base>;
using p_stmt_node_t = std::unique_ptr<stmt_node_base>;
using p_type_node_t = std::unique_ptr<type_node_base>;

struct modifiers {
  bool is_const;

  std::string to_string() const;
};

struct attribute {
  token identifier;
  std::vector<token> arguments;

  std::string to_string() const;
};

struct expr_node_base {
  size_t begin;
  size_t end;

  virtual vl_defdestructor(expr_node_base);

  virtual std::string to_string(uint32_t&) = 0;
  virtual p_expr_node_t clone() = 0;

  virtual void accept(node_visitor_base&, uint32_t) = 0;

  virtual p_type_node_t infer_type(trans_unit_context&) = 0;
  virtual int precedence() const {
    return 0;
  }
};

struct stmt_node_base {
  std::vector<attribute> attributes{};

  virtual vl_defdestructor(stmt_node_base);

  virtual std::string to_string(uint32_t&) = 0;
  virtual p_stmt_node_t clone() = 0;

  virtual void accept(node_visitor_base&) = 0;
};

struct type_node_base {
  expr_node_base* expression = nullptr;

  virtual vl_defdestructor(type_node_base);

  virtual std::string to_string(uint32_t&) = 0;
  virtual std::string to_string_x() = 0;
  virtual p_type_node_t clone() = 0;

  virtual void decay(node_visitor_base&, p_type_node_t&) {};
};

} // namespace via

#endif
