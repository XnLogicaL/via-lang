/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see
 * LICENSE for license information */

#pragma once

#include "arena.h"
#include "common.h"
#include "token.h"

// This value will be passed onto the arena allocator of the tokenizer
// Esentially the maximum amounts of
#ifndef VIA_LEXER_ALLOC_SIZE
    #define VIA_LEXER_ALLOC_SIZE 8 * 1024 * 1024 // 8 MiB
#endif

namespace via {

// Lexer class
// Tokenizes a string into tokens, cannot fail
// Can be used universally, has no dependencies, as seen in both viac and viaVM bytecode parser
class Tokenizer {
public:
    Tokenizer(ProgramData *program)
        : program(program)
    {
    }

    // Reads the source file and returns a source program
    // Which contains the original source string, tokenized source string and the file name
    void tokenize();

private:
    bool is_hex_char(char);
    size_t source_size();
    char peek(size_t ahead = 0);
    char consume(size_t ahead = 1);

    // Starts reading a "number" literal
    // Which can be a negative/positive floating point or integer
    Token read_number();
    // Reads an alpha-numeric identifier
    // Cannot start with a numeric character
    Token read_ident();
    // Reads a string literal that can be denoted with quotes
    // ! Does not support string interpolation
    Token read_string();
    // Reads and returns the current token
    Token get_token();

private:
    ProgramData *program;
    size_t pos = 0;
    size_t line = 1;
    size_t offset = 0;
};

VIA_INLINE std::vector<Token> fast_tokenize(std::string source)
{
    ProgramData program("<unknown>", source);
    Tokenizer tokenizer(&program);
    tokenizer.tokenize();

    if (!program.tokens) {
        return {};
    }

    return std::move(program.tokens->tokens);
}

} // namespace via
