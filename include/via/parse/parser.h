// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_PARSER_H
#define _VIA_PARSER_H

#include "stack.h"
#include "constant.h"
#include "bytecode.h"
#include "ast.h"
#include "common.h"
#include "ast-base.h"
#include "error-bus.h"

VIA_NAMESPACE_BEGIN

struct ParserError {
  size_t      where;
  std::string what;
};

class Parser final {
  public:
  template<typename T>
  using result = tl::expected<T, ParserError>;

  Parser(TransUnitContext& unit_ctx)
      : unit_ctx(unit_ctx) {}

  bool parse();

  private:
  TransUnitContext& unit_ctx;
  ErrorBus          err_bus;

  size_t position = 0;

  private:
  result<Token> current();
  result<Token> peek(int32_t ahead = 1);
  result<Token> consume(uint32_t ahead = 1);
  result<Token> expect_consume(TokenType type, const std::string& what);

  result<Modifiers> parse_modifiers();

  result<pTypeNode> parse_generic();
  result<pTypeNode> parse_type_primary();
  result<pTypeNode> parse_type();

  result<pExprNode> parse_primary();
  result<pExprNode> parse_postfix(pExprNode);
  result<pExprNode> parse_binary(int);
  result<pExprNode> parse_expr();

  result<pStmtNode> parse_declaration();
  result<pStmtNode> parse_scope();
  result<pStmtNode> parse_if();
  result<pStmtNode> parse_return();
  result<pStmtNode> parse_while();
  result<pStmtNode> parse_stmt();
};

VIA_NAMESPACE_END

#endif
