#ifndef VIA_LEXER_H
#define VIA_LEXER_H

#include "common.h"
#include "token.h"
#include "container.h"
#include "arena.hpp"

#ifndef __VIA_LEXER_ALLOC_SIZE
    #define __VIA_LEXER_ALLOC_SIZE 4 * 1024
#endif

namespace via
{

namespace Tokenization
{

class Tokenizer
{
    std::string& source;
    size_t pos;
    size_t line;
    size_t offset;
    ArenaAllocator m_alloc;

public:

    Tokenizer(std::string& source)
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

} // namespace Tokenization
    
} // namespace via

#endif // VIA_LEXER_H