// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_PARSER_H
#define VIA_HAS_HEADER_PARSER_H

#include "common.h"
#include "error-bus.h"
#include "ast.h"

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
  std::vector<Attribute> attrib_buffer;

private:
  // Returns the current Token in the Token buffer.
  result<Token> current();
  //
  result<Token> peek(int32_t ahead = 1);
  result<Token> consume(uint32_t ahead = 1);
  result<Token> expect_consume(TokenType type, const std::string& what);

  // Meta parsing.
  result<int> parse_modifiers();
  result<Attribute> parse_attribute();

  // Type parsing.
  result<TypeNode*> parse_generic();
  result<TypeNode*> parse_type_primary();
  result<TypeNode*> parse_type_postfix();
  result<TypeNode*> parse_type();

  // Expression parsing.
  result<ExprNode*> parse_primary();
  result<ExprNode*> parse_postfix(ExprNode*);
  result<ExprNode*> parse_binary(int);
  result<ExprNode*> parse_expr();

  // Statement parsing.
  result<StmtNode*> parse_declaration();
  result<StmtNode*> parse_scope();
  result<StmtNode*> parse_if();
  result<StmtNode*> parse_return();
  result<StmtNode*> parse_while();
  result<StmtNode*> parse_stmt();
};

} // namespace via

#endif
