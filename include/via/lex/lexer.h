// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_LEXER_H
#define _VIA_LEXER_H

#include "stack.h"
#include "constant.h"
#include "bytecode.h"
#include "ast.h"
#include "common.h"
#include "token.h"

VIA_NAMESPACE_BEGIN

// Lexer class
// Tokenizes a string into tokens, cannot fail
class Tokenizer {
public:
  Tokenizer(TransUnitContext& unit_ctx)
      : unit_ctx(unit_ctx) {}

  // Reads the source file and returns a source unit_ctx
  void tokenize();

private:
  bool   is_hex_char(char chr);
  size_t source_size();
  char   peek(size_t ahead = 0);
  char   consume(size_t ahead = 1);

  // Starts reading a number literal
  Token read_number(size_t);
  // Reads an alpha-numeric identifier
  // Cannot start with a numeric character
  Token read_ident(size_t);
  // Reads a string literal denoted with quotes
  Token read_string(size_t);
  // Reads and returns the current token
  Token get_token();

private:
  size_t            pos    = 0;
  size_t            line   = 1;
  size_t            offset = 0;
  TransUnitContext& unit_ctx;
};

VIA_INLINE std::vector<Token> fast_tokenize(std::string source) {
  TransUnitContext unit_ctx("<unknown>", source);

  Tokenizer tokenizer(unit_ctx);
  tokenizer.tokenize();

  return unit_ctx.tokens->get();
}

VIA_NAMESPACE_END

#endif
