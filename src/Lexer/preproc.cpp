/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "preproc.h"

namespace via
{

Token Preprocessor::consume(size_t ahead)
{
    size_t old_pos = pos;
    pos += ahead;
    return program.tokens->at(old_pos);
}

Token Preprocessor::peek(int ahead)
{
    return program.tokens->at(pos + ahead);
}

bool Preprocessor::preprocess()
{
    for (const Token &tok : *program.tokens)
    {
        if (tok.type == TokenType::KW_MACRO)
            parse_macro();
        else if (tok.type == TokenType::KW_DEFINE)
            parse_definition();
        else
        {
            pos++;
            continue;
        }
    }

    for (auto it : def_table)
    {
        expand_definition(it.second);
        erase_from_stream(it.second.begin, it.second.end);
    }

    for (auto it : macro_table)
    {
        expand_macro(it.second);
        erase_from_stream(it.second.begin, it.second.end);
    }

    return failed;
}

void Preprocessor::erase_from_stream(size_t begin, size_t end)
{
    auto tokens_begin = program.tokens->begin();
    program.tokens->erase(tokens_begin + begin, tokens_begin + end);
}

} // namespace via