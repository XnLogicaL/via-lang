#ifndef VIA_SYNTAX_ANALYZER_H
#define VIA_SYNTAX_ANALYZER_H

#include "common.h"
#include "highlighter.h"

#include "../token.h"
#include "../container.h"

#include "magic_enum/magic_enum.hpp"

namespace via
{

namespace Tokenization
{
        
namespace SyntaxAnalysis
{

class SyntaxAnalyzer
{
    viaSourceContainer& container;
    size_t pos = 0;
    bool failed = false;

    Token peek(size_t ahead = 0);
    Token consume(size_t ahead = 1);

    bool is_valid_expression();
    bool is_valid_term();
    bool is_valid_type();

    void check_fun_call();
    void check_argument();
    void check_ident_token();
    void check_spec_char();
    bool is_special_character(TokenType type);
    void check_decl();
    void check_ret();
    void check_scope();
    void check_func();
    void check_structure();

    void match();
    void check_invalid_token();  // Declare if necessary

public:

    explicit SyntaxAnalyzer(viaSourceContainer& container)
        : container(container) {}

    bool analyze();
};

} // namespace SyntaxAnalysis

} // namespace Tokenization

} // namespace via

#endif // VIA_SYNTAX_ANALYZER_H