// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "preproc.h"
#include "lexer.h"

namespace via {

void Preprocessor::declare_macro(Macro mac)
{
    macro_table[mac.name] = mac;
}

void Preprocessor::declare_definition(Definition def)
{
    def_table[def.identifier] = def;
}

void Preprocessor::declare_default()
{
    declare_definition({
        "__version__",
        fast_tokenize(VIA_VERSION),
        0,
        0,
        0,
    });

    declare_definition({
        "__file__",
        fast_tokenize(program->file),
        0,
        0,
        0,
    });

    declare_macro({
        "printf",
        {"__fmt", "..."},
        fast_tokenize("print(string.format(__fmt, ...))"),
        0,
        0,
        0,
    });
}

Token Preprocessor::consume(size_t ahead)
{
    size_t old_pos = pos;
    pos += ahead;
    return program->tokens->tokens.at(old_pos);
}

Token Preprocessor::peek(int ahead)
{
    return program->tokens->tokens.at(pos + ahead);
}

bool Preprocessor::preprocess()
{
    for (const Token &tok : program->tokens->tokens) {
        if (tok.type == TokenType::KW_MACRO) {
            parse_macro();
        }
        else if (tok.type == TokenType::KW_DEFINE) {
            parse_definition();
        }
        else {
            pos++;
            continue;
        }
    }

    for (auto it : def_table) {
        expand_definition(it.second);
        erase_from_stream(it.second.begin, it.second.end);
    }

    for (auto it : macro_table) {
        expand_macro(it.second);
        erase_from_stream(it.second.begin, it.second.end);
    }

    return failed;
}

void Preprocessor::erase_from_stream(size_t begin, size_t end)
{
    auto tokens_begin = program->tokens->tokens.begin();
    program->tokens->tokens.erase(tokens_begin + begin, tokens_begin + end);
}

} // namespace via