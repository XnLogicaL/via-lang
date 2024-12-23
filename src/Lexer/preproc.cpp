/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "preproc.h"

namespace via::Tokenization
{

Token Preprocessor::consume(size_t ahead)
{
    size_t old_pos = pos;
    pos += ahead;
    return container.tokens.at(old_pos);
}

Token Preprocessor::peek(int ahead)
{
    return container.tokens.at(pos + ahead);
}

bool Preprocessor::preprocess()
{
    for (const Token &tok : container.tokens)
    {
        if (tok.type == TokenType::KW_MACRO)
        {
            Macro mac = parse_macro();
            expand_macro(mac);
        }
        else if (tok.type == TokenType::KW_DEFINE)
        {
            Definition def = parse_definition();
            expand_definition(def);
        }
        else
        {
            pos++;
            continue;
        }
    }

    for (auto it : macro_table)
        erase_from_stream(it.second.begin, it.second.end);

    for (auto it : def_table)
        erase_from_stream(it.second.begin, it.second.end);

    return failed;
}

void Preprocessor::erase_from_stream(size_t begin, size_t end)
{
    auto tokens_begin = container.tokens.begin();
    container.tokens.erase(tokens_begin + begin, tokens_begin + end);
}

} // namespace via::Tokenization