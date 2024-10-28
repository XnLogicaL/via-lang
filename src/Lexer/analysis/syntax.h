#ifndef VIA_SYNTAX_ANALYZER_H
#define VIA_SYNTAX_ANALYZER_H

#include "common.h"

#include "../token.h"
#include "../container.h"
#include "../util/highlighter.hpp"

#include "magic_enum/magic_enum.hpp"

namespace via
{

namespace Tokenization
{
        
namespace SyntaxAnalysis
{

class SyntaxAnalyzer
{
    viaSourceContainer container;
    bool failed;
    size_t pos;

public:

    SyntaxAnalyzer(viaSourceContainer& container)
        : container(container)
        , pos(0)
        , failed(false) {}

    Token peek(size_t ahead = 0);
    Token consume(size_t ahead = 1);

    void expect(std::vector<TokenType> expected_types);
    void match();

    bool analyze();

    bool is_valid_expression();
    bool is_valid_term();

    void check_invalid_token();
    void check_ident_token();
    void check_fun_call();
    void check_argument();
    void check_spec_char();
    void check_literal();
};

bool analyze(viaSourceContainer& container);

} // namespace SyntaxAnalysis

} // namespace Tokenization

} // namespace via

#endif // VIA_SYNTAX_ANALYZER_H