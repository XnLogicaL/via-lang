/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <vector>
#include <via/config.hpp>
#include "source.hpp"
#include "support/memory.hpp"
#include "token.hpp"

namespace via {

using TokenTree = std::vector<Token*>;

class Lexer final
{
  public:
    Lexer(const SourceBuffer& source)
        : m_source(source),
          m_cursor(source.begin()),
          m_end(source.end() - 1)
    {}

  public:
    TokenTree tokenize();

  private:
    char advance(size_t ahead = 1);
    char peek(ssize_t ahead = 0);
    Token* read_number();
    Token* read_string();
    Token* read_operator();
    Token* read_identifier();
    bool skip_comment();

  private:
    ScopedAllocator m_alloc;
    const SourceBuffer& m_source;
    const char* m_cursor;
    const char* m_end;
};

std::string to_string(const TokenTree& tt);

} // namespace via
