// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_LEXER_H
#define VIA_HAS_HEADER_LEXER_H

#include "common.h"
#include "context.h"
#include "token.h"

namespace via {

// Lexer class
// Tokenizes a String into tokens, cannot fail
class Lexer {
public:
  Lexer(TransUnitContext& unit_ctx)
    : unit_ctx(unit_ctx) {}

  // Reads the source file and returns a source unit_ctx
  void tokenize();

private:
  bool is_hex_char(char chr);
  size_t source_size();
  char peek(size_t ahead = 0);
  char consume(size_t ahead = 1);

  // Starts reading a number literal
  Token read_number(size_t);
  // Reads an alpha-numeric identifier
  // Cannot start with a numeric character
  Token read_ident(size_t);
  // Reads a String literal denoted with quotes
  Token read_string(size_t);
  // Reads and returns the current Token
  Token get_token();

private:
  size_t pos = 0;
  size_t line = 1;
  size_t offset = 0;
  
  TransUnitContext& unit_ctx;
};

inline std::vector<Token> fast_tokenize(std::string source) {
  TransUnitContext unit_ctx("<unknown>", source);

  Lexer tokenizer(unit_ctx);
  tokenizer.tokenize();

  return unit_ctx.tokens->get();
}

} // namespace via

#endif
