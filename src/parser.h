// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_PARSER_H
#define VIA_HAS_HEADER_PARSER_H

#include "common.h"
#include "error-bus.h"
#include "sema_utils.h"
#include "ast.h"

namespace via {

struct ParserError : public std::exception {
  const bool flat;
  const LexLocation loc;
  const std::string message;

  explicit ParserError(bool flat, const LexLocation loc, const std::string message)
    : flat(flat),
      loc(loc),
      message(message) {}
};

// Parser class. Iterates over tokens and builds an AST.
class Parser final {
public:
  // Constructor
  Parser(Context& lctx)
    : lctx(lctx) {}

  // Parser entry point. Returns a fail status.
  bool parse();
  AstNode* alloc_node(LexLocation loc, AstKind kind, AstNode::Un u);

private:
  size_t position = 0;
  CErrorBus err_bus;
  Context& lctx;
  std::vector<Attribute> attrib_buffer;

private:
  Token* current();
  Token* peek(int32_t ahead = 1);
  Token* consume(uint32_t ahead = 1);
  Token* expect_consume(TokenType type, const std::string& what);

  // Meta parsing.
  int* parse_modifiers();
  Attribute* parse_attribute();

  // Type parsing.
  AstNode* parse_generic();
  AstNode* parse_type_primary();
  AstNode* parse_type_postfix();
  AstNode* parse_type();

  // Expression parsing.
  AstNode* parse_primary();
  AstNode* parse_postfix(AstNode*);
  AstNode* parse_binary(int);
  AstNode* parse_expr();

  // Statement parsing.
  AstNode* parse_declaration();
  AstNode* parse_scope();
  AstNode* parse_if();
  AstNode* parse_return();
  AstNode* parse_while();
  AstNode* parse_stmt();
};

} // namespace via

#endif
