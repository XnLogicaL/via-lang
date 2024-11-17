/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"

namespace via::Tokenization
{

enum class TokenType {
    // Keywords
    KW_DO,
    KW_IN,
    KW_LOCAL,
    KW_GLOBAL,
    KW_AS,
    KW_IF,
    KW_ELSE,
    KW_ELIF,
    KW_WHILE,
    KW_FOR,
    KW_RETURN,
    KW_FUNC,
    KW_VAR,
    KW_CONST,
    KW_NEW,
    KW_BREAK,
    KW_CONTINUE, // continue
    KW_SWITCH,   // switch
    KW_CASE,     // case
    KW_DEFAULT,  // default
    KW_DELETE,   // delete
    KW_AND,      // and
    KW_NOT,      // not
    KW_OR,       // or
    KW_STRUCT,
    KW_NAMESPACE,
    KW_PROPERTY,
    KW_IMPORT,
    KW_EXPORT,
    KW_MACRO,
    KW_DEFINE,

    // Operators
    OP_EXP, // ^
    OP_ADD, // +
    OP_SUB, // -
    OP_MUL, // *
    OP_DIV, // /
    OP_MOD, // %
    OP_EQ,  // ==
    OP_NEQ, // !=
    OP_LT,  // <
    OP_GT,  // >
    OP_LEQ, // <=
    OP_GEQ, // >=
    OP_INC, // ++
    OP_DEC, // --
    OP_ASGN,

    // Literals
    LIT_INT,    // Integer literals
    LIT_FLOAT,  // Floating-point literals
    LIT_STRING, // String literals
    LIT_CHAR,   // Character literals
    LIT_BOOL,   // Boolean literals

    // Identifiers
    IDENTIFIER, // Variable and function names

    // Punctuation
    PAREN_OPEN,    // (
    PAREN_CLOSE,   // )
    BRACE_OPEN,    // {
    BRACE_CLOSE,   // }
    BRACKET_OPEN,  // [
    BRACKET_CLOSE, // ]
    COMMA,         // ,
    SEMICOLON,     // ;
    COLON,         // :
    DOT,           // .

    // Miscellaneous
    AT,           // @
    TILDE,        // ~
    QUOTE,        // '
    PIPE,         // |
    DOLLAR,       // $
    BACKTICK,     // `
    AMPERSAND,    // &
    DOUBLE_QUOTE, // "
    EXCLAMATION,  // !

    EOF_,   // End of file
    UNKNOWN // Unknown token
};

struct Token
{
    TokenType type;
    std::string value;
    size_t line;
    size_t offset;
    bool has_thrown_error = false;

    std::string to_string() const noexcept;
    bool is_literal() const noexcept;
    bool is_operator() const noexcept;
    int bin_prec() const noexcept;
};

} // namespace via::Tokenization