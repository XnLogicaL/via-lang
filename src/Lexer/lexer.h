/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "token.h"
#include "container.h"
#include "arena.hpp"

#ifndef __VIA_LEXER_ALLOC_SIZE
    #define __VIA_LEXER_ALLOC_SIZE 8 * 1024 * 1024
#endif

namespace via::Tokenization
{

class Tokenizer
{
private:
    std::string &source;
    size_t pos;
    size_t line;
    size_t offset;
    ArenaAllocator m_alloc;

public:

    Tokenizer(std::string &source)
        : source(source)
        , pos(0)
        , line(1)
        , offset(0)
        , m_alloc(__VIA_LEXER_ALLOC_SIZE) {}

    Token read_number() noexcept;
    Token read_ident() noexcept;
    Token read_string() noexcept;

    Token get_token() noexcept;
    viaSourceContainer tokenize() noexcept;
};

} // namespace via::Tokenization