// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _vl_parser_h
#define _vl_parser_h

#include "stack.h"
#include "constant.h"
#include "bytecode.h"
#include "ast.h"
#include "common.h"
#include "ast-base.h"
#include "error-bus.h"

namespace via {

struct parse_error {
  size_t where;
  std::string what;
};

class parser final {
public:
  template<typename T>
  using result = tl::expected<T, parse_error>;

  parser(trans_unit_context& unit_ctx)
    : unit_ctx(unit_ctx) {}

  bool parse();

private:
  trans_unit_context& unit_ctx;
  error_bus err_bus;

  size_t position = 0;

  std::vector<attribute> attrib_buffer;

private:
  result<token> current();
  result<token> peek(int32_t ahead = 1);
  result<token> consume(uint32_t ahead = 1);
  result<token> expect_consume(token_type type, const std::string& what);

  result<modifiers> parse_modifiers();
  result<attribute> parse_attribute();

  result<p_type_node_t> parse_generic();
  result<p_type_node_t> parse_type_primary();
  result<p_type_node_t> parse_type();

  result<p_expr_node_t> parse_primary();
  result<p_expr_node_t> parse_postfix(p_expr_node_t);
  result<p_expr_node_t> parse_binary(int);
  result<p_expr_node_t> parse_expr();

  result<p_stmt_node_t> parse_declaration();
  result<p_stmt_node_t> parse_scope();
  result<p_stmt_node_t> parse_if();
  result<p_stmt_node_t> parse_return();
  result<p_stmt_node_t> parse_while();
  result<p_stmt_node_t> parse_stmt();
};

} // namespace via

#endif
