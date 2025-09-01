// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_TOKEN_H_
#define VIA_CORE_TOKEN_H_

#include <fmt/ranges.h>
#include <via/config.h>
#include <via/types.h>
#include "location.h"

namespace via
{

struct Token
{
  enum class Kind
  {
    EOF_ = 0,  // end of file
    ILLEGAL,   // unrecognized lexeme

    IDENTIFIER,        // identifier
    IDENTIFIER_MACRO,  // macro identifier
    LIT_NIL,           // nil
    LIT_INT,           // integer literal
    LIT_BINT,          // binary integer literal
    LIT_XINT,          // hexadecimal integer literal
    LIT_FLOAT,         // floating point literal
    LIT_TRUE,          // true literal
    LIT_FALSE,         // false literal
    LIT_STRING,

    KW_VAR,     // var
    KW_CONST,   // const
    KW_FN,      // fn
    KW_TYPE,    // type
    KW_WHILE,   // while
    KW_FOR,     // for
    KW_IF,      // if
    KW_IN,      // in
    KW_OF,      // of
    KW_ELSE,    // else
    KW_DO,      // do
    KW_AND,     // and
    KW_OR,      // or
    KW_NOT,     // not
    KW_RETURN,  // return
    KW_AS,      // as
    KW_IMPORT,  // import
    KW_MODULE,  // mod
    KW_STRUCT,  // struct
    KW_ENUM,    // enum
    KW_USING,   // using
    KW_BOOL,    // bool
    KW_INT,     // int
    KW_FLOAT,   // float
    KW_STRING,  // string

    PERIOD,       // .
    COMMA,        // ,
    SEMICOLON,    // ;
    COLON,        // :
    COLON_COLON,  // ::
    ARROW,        // ->
    QUESTION,     // ?

    PAREN_OPEN,     // (
    PAREN_CLOSE,    // )
    BRACKET_OPEN,   // [
    BRACKET_CLOSE,  // ]
    CURLY_OPEN,     // {
    CURLY_CLOSE,    // }

    OP_PLUS,       // +
    OP_MINUS,      // -
    OP_STAR,       // *
    OP_SLASH,      // /
    OP_STAR_STAR,  // **
    OP_PERCENT,    // %
    OP_AMP,        // &
    OP_TILDE,      // ~
    OP_CARET,      // ^
    OP_PIPE,       // |
    OP_SHL,        // <<
    OP_SHR,        // >>
    OP_BANG,       // !
    OP_LT,         // <
    OP_GT,         // >
    OP_DOT_DOT,    // ..

    OP_PLUS_PLUS,    // ++
    OP_MINUS_MINUS,  // --

    OP_EQ,            // =
    OP_EQ_EQ,         // ==
    OP_PLUS_EQ,       // +=
    OP_MINUS_EQ,      // -=
    OP_STAR_EQ,       // *=
    OP_SLASH_EQ,      // /=
    OP_STAR_STAR_EQ,  // **=
    OP_PERCENT_EQ,    // %=
    OP_AMP_EQ,        // &=
    OP_CARET_EQ,      // ^=
    OP_PIPE_EQ,       // |=
    OP_SHL_EQ,        // <<=
    OP_SHR_EQ,        // >>=
    OP_BANG_EQ,       // !=
    OP_LT_EQ,         // <=
    OP_GT_EQ,         // >=
    OP_DOT_DOT_EQ,    // ..=
  } kind;

  const char* lexeme;
  usize size;

  String dump() const;
  String toString() const { return String(lexeme, size); }
  StringView toStringView() const { return StringView(lexeme, size); }
  SourceLoc location(const String& source) const;
};

inline String toString(const Token& tok)
{
  return tok.dump();
}

}  // namespace via

#endif
