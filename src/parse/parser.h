// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_PARSER_H
#define VIA_HAS_HEADER_PARSER_H

#include "common.h"
#include "error-bus.h"
#include "ast.h"
#include "ast-base.h"

namespace via {

// Parse error object that holds error location and error message.
struct ParseError {
  size_t where;
  std::string what;
};

// Parser class. Iterates over tokens and builds an AST.
class Parser final {
public:
  template<typename T>
  using result = tl::expected<T, ParseError>;

  // Constructor
  Parser(TransUnitContext& unit_ctx)
    : unit_ctx(unit_ctx) {}

  // Parser entry point. Returns a fail status.
  bool parse();

private:
  // Position in the Token buffer.
  size_t position = 0;
  CErrorBus err_bus;
  TransUnitContext& unit_ctx;
  // Attribute buffer, holds attributes of statements and gets cleared every time a new statement is
  // parsed.
  std::vector<StmtAttribute> attrib_buffer;

private:
  // Returns the current Token in the Token buffer.
  result<Token> current();
  //
  result<Token> peek(int32_t ahead = 1);
  result<Token> consume(uint32_t ahead = 1);
  result<Token> expect_consume(TokenType type, const std::string& what);

  // Meta parsing.
  result<StmtModifiers> parse_modifiers();
  result<StmtAttribute> parse_attribute();

  // Type parsing.
  result<TypeNodeBase*> parse_generic();
  result<TypeNodeBase*> parse_type_primary();
  result<TypeNodeBase*> parse_type();

  // Expression parsing.
  result<ExprNodeBase*> parse_primary();
  result<ExprNodeBase*> parse_postfix(ExprNodeBase*);
  result<ExprNodeBase*> parse_binary(int);
  result<ExprNodeBase*> parse_expr();

  // Statement parsing.
  result<StmtNodeBase*> parse_declaration();
  result<StmtNodeBase*> parse_scope();
  result<StmtNodeBase*> parse_if();
  result<StmtNodeBase*> parse_return();
  result<StmtNodeBase*> parse_while();
  result<StmtNodeBase*> parse_stmt();
};

} // namespace via

#endif
