//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_PARSER_H
#define VIA_HAS_HEADER_PARSER_H

#include "stack.h"
#include "constant.h"
#include "bytecode.h"
#include "ast.h"
#include "common.h"
#include "ast-base.h"
#include "error-bus.h"

namespace via {

// Parse error object that holds error location and error message.
struct parse_error {
  size_t where;
  std::string what;
};

// Parser class. Iterates over tokens and builds an AST.
class parser final {
public:
  template<typename T>
  using result = tl::expected<T, parse_error>;

  // Constructor
  parser(trans_unit_context& unit_ctx)
    : unit_ctx(unit_ctx) {}

  // Parser entry point. Returns a fail status.
  bool parse();

private:
  // Position in the token buffer.
  size_t position = 0;
  error_bus err_bus;
  trans_unit_context& unit_ctx;
  // Attribute buffer, holds attributes of statements and gets cleared every time a new statement is
  // parsed.
  std::vector<attribute> attrib_buffer;

private:
  // Returns the current token in the token buffer.
  result<token> current();
  //
  result<token> peek(int32_t ahead = 1);
  result<token> consume(uint32_t ahead = 1);
  result<token> expect_consume(token_type type, const std::string& what);

  // Meta parsing.
  result<modifiers> parse_modifiers();
  result<attribute> parse_attribute();

  // Type parsing.
  result<p_type_node_t> parse_generic();
  result<p_type_node_t> parse_type_primary();
  result<p_type_node_t> parse_type();

  // Expression parsing.
  result<p_expr_node_t> parse_primary();
  result<p_expr_node_t> parse_postfix(p_expr_node_t);
  result<p_expr_node_t> parse_binary(int);
  result<p_expr_node_t> parse_expr();

  // Statement parsing.
  result<p_stmt_node_t> parse_declaration();
  result<p_stmt_node_t> parse_scope();
  result<p_stmt_node_t> parse_if();
  result<p_stmt_node_t> parse_return();
  result<p_stmt_node_t> parse_while();
  result<p_stmt_node_t> parse_stmt();
};

} // namespace via

#endif
