// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_LEXER_H_
#define VIA_CORE_LEXER_H_

#include <via/config.h>
#include <via/types.h>
#include "memory.h"
#include "token.h"

namespace via {

class Lexer final {
 public:
  Lexer(const Vec<char>& file) : m_file(file), m_cursor(file.data()) {}

 public:
  Vec<Token*> tokenize();

 private:
  char advance(int ahead = 1);
  char peek(int ahead = 0);
  Token* read_number();
  Token* read_string();
  Token* read_symbol();
  Token* read_identifier();
  bool skip_comment();

 private:
  const Vec<char>& m_file;
  const char* m_cursor;
  Allocator m_alloc;
};

}  // namespace via

#endif
