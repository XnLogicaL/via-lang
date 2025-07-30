// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_LEXER_H_
#define VIA_CORE_LEXER_H_

#include <mimalloc.h>
#include <util/buffer.h>
#include <util/memory.h>
#include <via/config.h>
#include "token.h"

namespace via {

namespace core {

namespace lex {

class Lexer final {
 public:
  Lexer(const FileBuf& file) : file(file) {}

  TokenBuf tokenize();

 private:
  char advance();
  char peek(int ahead);

  Token* read_number();
  Token* read_string();
  Token* read_symbol();
  Token* read_identifier();

  bool skip_comment();

 private:
  const FileBuf& file;
  HeapAllocator alloc;
};

void dump_ttree(const TokenBuf& B);

}  // namespace lex

}  // namespace core

}  // namespace via

#endif
