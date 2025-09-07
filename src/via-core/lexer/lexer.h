// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_LEXER_H_
#define VIA_CORE_LEXER_H_

#include <via/config.h>
#include <via/types.h>
#include "memory.h"
#include "token.h"

namespace via
{

using TokenTree = Vec<Token*>;

class Lexer final
{
 public:
  Lexer(const std::string& file)
      : mFile(file), mCursor(file.data()), mEnd(file.data() + file.size() - 1)
  {}

 public:
  TokenTree tokenize();

 private:
  char advance(int ahead = 1);
  char peek(int ahead = 0);
  Token* readNumber();
  Token* readString();
  Token* readSymbol();
  Token* readIdentifier();
  bool skipComment();

 private:
  const std::string& mFile;
  const char* mCursor;
  const char* mEnd;
  Allocator mAlloc;
};

namespace debug
{

[[nodiscard]] std::string dump(const TokenTree& tt);

}

}  // namespace via

#endif
