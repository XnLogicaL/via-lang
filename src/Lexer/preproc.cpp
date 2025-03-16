// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "preproc.h"
#include "lexer.h"

VIA_NAMESPACE_BEGIN

using enum OutputSeverity;
using enum TokenType;

void Preprocessor::declare_macro(Macro mac) {
    macro_table[mac.name] = mac;
}

void Preprocessor::declare_definition(Definition def) {
    def_table[def.identifier] = def;
}

void Preprocessor::declare_default() {
    declare_definition({
        0,
        0,
        0,
        "__version__",
        fast_tokenize(VIA_VERSION),
    });

    declare_definition({
        0,
        0,
        0,
        "__file__",
        fast_tokenize(program.file),
    });

    declare_definition({
        0,
        0,
        0,
        "__source__",
        ({
            TokenStream& stream = *program.token_stream;
            stream.get();
        }),
    });
}

Token Preprocessor::consume(size_t ahead) {
    pos += ahead;
    return program.token_stream->at(pos - ahead);
}

Token Preprocessor::peek(int ahead) {
    return program.token_stream->at(pos + ahead);
}

void Preprocessor::handle_pragma() {
    consume(); // Consume 'pragma'

    Token id = consume(); // Consume directive id

    std::vector<Token> token_buffer;

    // If the pragma is followed by parentheses, we process the parameters
    if (peek().type == PAREN_OPEN) {
        consume(); // Consume '('

        while (peek().type != PAREN_CLOSE) {
            if (peek().type == EOF_) {
                PREPROCESSOR_ERROR(
                    pos, "Unexpected end of file while processing pragma parameters"
                );
            }

            token_buffer.push_back(consume());
        }

        consume(); // Consume ')'
    }

    if (id.lexeme == "optimize") {
        if (token_buffer.empty()) {
            PREPROCESSOR_ERROR(pos - 2, "Expected optimization level");
        }

        Token optimization_level = token_buffer.at(0);

        if (optimization_level.type != LIT_INT) {
            PREPROCESSOR_ERROR(
                pos - token_buffer.size() - 2, "Expected integer as optimization level"
            );
        }

        program.optimization_level = std::stoul(optimization_level.lexeme);
    }
    else {
        PREPROCESSOR_ERROR(
            pos - token_buffer.size() - 2, std::format("Unknown pragma directive '{}'", id.lexeme)
        );
    }
}

bool Preprocessor::preprocess() {
    try {
        for (size_t i = 0; i < program.token_stream->size(); i++) {
            Token tok = program.token_stream->at(i);

            switch (tok.type) {
            case KW_PRAGMA:
                handle_pragma();
                break;
            case KW_MACRO:
                parse_macro();
                break;
            case KW_DEFINE:
                parse_definition();
                break;
            default:
                break;
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
    }
    catch (const PreprocessorError& e) {
        emitter.out(program.token_stream->at(pos), e.what(), Error);
        return true;
    }

    return false;
}

void Preprocessor::erase_from_stream(size_t begin, size_t end) {
    auto& tok_vec = program.token_stream->get();

    auto tokens_begin = tok_vec.begin();
    tok_vec.erase(tokens_begin + begin, tokens_begin + end);
}

VIA_NAMESPACE_END
