/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <vector>
#include <via/config.h>
#include <via/types.h>
#include "support/memory.h"
#include "token.h"

namespace via {

using TokenTree = std::vector<Token*>;

class Lexer final
{
  public:
    Lexer(const std::string& source)
        : m_source(source),
          m_cursor(source.data()),
          m_end(source.data() + source.size() - 1)
    {}

  public:
    TokenTree tokenize();

  private:
    char advance(isize ahead = 1);
    char peek(isize ahead = 0);
    Token* read_number();
    Token* read_string();
    Token* read_operator();
    Token* read_identifier();
    bool skip_comment();

  private:
    ScopedAllocator m_alloc;
    const std::string& m_source;
    const char* m_cursor;
    const char* m_end;
};

namespace debug {

[[nodiscard]] std::string get_dump(const TokenTree& tt);

}
} // namespace via
