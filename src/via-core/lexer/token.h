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

    IDENT,  // identifier
    STRING,
    NIL,     // nil
    MIDENT,  // macro identifier
    INT,     // integer literal
    BINT,    // binary integer literal
    XINT,    // hexadecimal integer literal
    FP,      // floating point literal
    TRUE,    // true literal
    FALSE,   // false literal

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
    KW_SHL,     // shl
    KW_SHR,     // shr
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

    DOT,                // .
    COMMA,              // ,
    SEMICOLON,          // ;
    COLON,              // :
    DBCOLON,            // ::
    ARROW,              // ->
    QUESTION,           // ?
    PLUS,               // +
    MINUS,              // -
    ASTERISK,           // *
    FSLASH,             // /
    POW,                // **
    PERCENT,            // %
    AMPERSAND,          // &
    TILDE,              // ~
    CARET,              // ^
    PIPE,               // |
    BANG,               // !
    INC,                // ++
    DEC,                // --
    LESSTHAN,           // <
    GREATERTHAN,        // >
    CONCAT,             // ..
    LPAREN,             // (
    RPAREN,             // )
    LBRACKET,           // [
    RBRACKET,           // ]
    LCURLY,             // {
    RCURLY,             // }
    EQUALS,             // =
    DBEQUALS,           // ==
    PLUSEQUALS,         // +=
    MINUSEQUALS,        // -=
    ASTERISKEQUALS,     // *=
    FSLASHEQUALS,       // /=
    POWEQUALS,          // **=
    PERCENTEQUALS,      // %=
    AMPERSANDEQUALS,    // &=
    CARETEQUALS,        // ^=
    PIPEEQUALS,         // |=
    BANGEQUALS,         // !=
    LESSTHANEQUALS,     // <=
    GREATERTHANEQUALS,  // >=
    CONCATEQUALS,       // ..=
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
