/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "arena.hpp"
#include "common.h"
#include "token.h"

// This value will be passed onto the arena allocator of the tokenizer
// Esentially the maximum amounts of
#ifndef VIA_LEXER_ALLOC_SIZE
    #define VIA_LEXER_ALLOC_SIZE 8 * 1024 * 1024 // 8 MiB
#endif

namespace via
{

// Lexer class
// Tokenizes a string into tokens, cannot fail
// Can be used universally, has no dependencies, as seen in both viac and viaVM bytecode parser
class Tokenizer
{
public:
    Tokenizer(ProgramData &program)
        : program(program)
        , pos(0)
        , line(1)
        , offset(0)
        , alloc(VIA_LEXER_ALLOC_SIZE)
    {
    }

    // Reads the source file and returns a source program
    // Which contains the original source string, tokenized source string and the file name
    void tokenize();

private:
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
    ProgramData &program;
    size_t pos;
    size_t line;
    size_t offset;
    ArenaAllocator alloc;
};

} // namespace via
