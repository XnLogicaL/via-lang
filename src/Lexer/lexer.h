#ifndef VIA_LEXER_H
#define VIA_LEXER_H

#include "common.h"
#include "token.h"
#include "container.h"

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

public:

    Tokenizer(std::string& source)
        : source(source)
        , pos(0)
        , line(1)
        , offset(0) {}

    Token read_number() noexcept;
    Token read_ident() noexcept;
    Token read_string() noexcept;

    Token get_token() noexcept;
    viaSourceContainer tokenize() noexcept;
};

viaSourceContainer tokenize_code(std::string& source);

} // namespace Tokenization
    
} // namespace via

#endif // VIA_LEXER_H