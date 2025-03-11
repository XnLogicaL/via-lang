// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_LEXER_H
#define _VIA_LEXER_H

#include "arena.h"
#include "common.h"
#include "token.h"

VIA_NAMESPACE_BEGIN

// Lexer class
// Tokenizes a string into tokens, cannot fail
// Can be used universally, has no dependencies, as seen in both viac and viaVM bytecode parser
class Tokenizer {
public:
    Tokenizer(ProgramData& program)
        : program(program) {}

    // Reads the source file and returns a source program
    // Which contains the original source string, tokenized source string and the file name
    void tokenize();

private:
    bool   is_hex_char(char chr);
    size_t source_size();
    char   peek(size_t ahead = 0);
    char   consume(size_t ahead = 1);

    // Starts reading a "number" literal
    // Which can be a negative/positive floating point or integer
    Token read_number(size_t);
    // Reads an alpha-numeric identifier
    // Cannot start with a numeric character
    Token read_ident(size_t);
    // Reads a string literal that can be denoted with quotes
    Token read_string(size_t);
    // Reads and returns the current token
    Token get_token();

private:
    size_t       pos    = 0;
    size_t       line   = 1;
    size_t       offset = 0;
    ProgramData& program;
};

VIA_INLINE std::vector<Token> fast_tokenize(std::string source) {
    ProgramData program("<unknown>", source);
    Tokenizer   tokenizer(program);
    tokenizer.tokenize();
    return program.tokens->tokens;
}

VIA_NAMESPACE_END

#endif
