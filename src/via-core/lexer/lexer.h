/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

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
  char advance(isize ahead = 1);
  char peek(isize ahead = 0);
  Token* readNumber();
  Token* readString();
  Token* readSymbol();
  Token* readIdentifier();
  bool skipComment();

 private:
  Allocator mAlloc;
  const std::string& mFile;
  const char* mCursor;
  const char* mEnd;
};

namespace debug
{

[[nodiscard]] std::string dump(const TokenTree& tt);

}

}  // namespace via
