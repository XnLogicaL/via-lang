/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <string>

namespace via
{

enum class TokenType
{
    // Keywords
    KW_DO,        // do
    KW_IN,        // in
    KW_LOCAL,     // local
    KW_GLOBAL,    // global
    KW_AS,        // as
    KW_IF,        // if
    KW_ELSE,      // else
    KW_ELIF,      // elif
    KW_WHILE,     // while
    KW_FOR,       // for
    KW_RETURN,    // return
    KW_FUNC,      // func
    KW_VAR,       // var
    KW_CONST,     // const
    KW_NEW,       // new
    KW_BREAK,     // break
    KW_CONTINUE,  // continue
    KW_SWITCH,    // switch
    KW_CASE,      // case
    KW_DEFAULT,   // default
    KW_DELETE,    // delete
    KW_AND,       // and
    KW_NOT,       // not
    KW_OR,        // or
    KW_STRUCT,    // struct
    KW_NAMESPACE, // namespace
    KW_PROPERTY,  // property
    KW_IMPORT,    // import
    KW_EXPORT,    // export
    KW_MACRO,     // macro
    KW_DEFINE,    // define

    // Operators
    OP_EXP,  // ^
    OP_ADD,  // +
    OP_SUB,  // -
    OP_MUL,  // *
    OP_DIV,  // /
    OP_MOD,  // %
    OP_EQ,   // ==
    OP_NEQ,  // !=
    OP_LT,   // <
    OP_GT,   // >
    OP_LEQ,  // <=
    OP_GEQ,  // >=
    OP_INC,  // ++
    OP_DEC,  // --
    OP_ASGN, // =

    // Literals
    LIT_INT,    // Integer literals
    LIT_FLOAT,  // Floating-point literals
    LIT_STRING, // String literals
    LIT_BOOL,   // Boolean literals
    LIT_NIL,    // Nil literal (just "nil" lol)

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
    QUESTION,     // ?

    EOF_,   // End of file
    UNKNOWN // Unknown token
};

struct Token
{
    TokenType type;
    std::string value;
    size_t line;
    size_t offset;
    // This is an internal value used by the syntax analyzer
    // To determine wether or not there was an error thrown on this token
    // This makes it so that the console doesn't get flooded by errors
    bool has_thrown_error = false;

    // Returns a stringified version of the token
    std::string to_string() const noexcept;
    // Returns if the token is a literal
    bool is_literal() const noexcept;
    // Returns if the token is an operator
    bool is_operator() const noexcept;
    // Returns the tokens binary precedence
    // Returns -1 if invalid, eg. if not an operator
    int bin_prec() const noexcept;
};

} // namespace via