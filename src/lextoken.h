// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_LEXTOKEN_H
#define VIA_LEXTOKEN_H

#include "common.h"
#include "heapbuf.h"

namespace via {

namespace lex {

struct State;

enum TokenKind {
  TK_EOF = 0, // end of file
  TK_ILLEGAL, // unrecognized lexeme

  TK_IDENT, // identifier
  TK_STRING,
  TK_MIDENT, // macro identifier
  TK_INT,    // integer literal
  TK_BINT,   // binary integer literal
  TK_XINT,   // hexadecimal integer literal
  TK_FP,     // floating point literal

  TK_DOT, // .
};

struct Location {
  const size_t begin;
  const size_t end;
};

struct Token {
  TokenKind kind;
  const char* lexeme;
  size_t size;
};

using TokenBuf = HeapBuffer<Token*>;

Location location(State* L, Token* token);

} // namespace lex

} // namespace via

#endif
