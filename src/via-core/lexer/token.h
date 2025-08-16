// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_TOKEN_H_
#define VIA_CORE_TOKEN_H_

#include <via/config.h>
#include <magic_enum/magic_enum.hpp>
#include "buffer.h"
#include "location.h"

namespace via {

namespace core {

namespace lex {
struct Token;
}

using TokenBuf = Buffer<lex::Token*>;

namespace lex {

enum class TokenKind {
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

  KW_VAR,    // var
  KW_MACRO,  // macro
  KW_FUNC,   // func
  KW_TYPE,   // type
  KW_WHILE,  // while
  KW_FOR,    // for
  KW_IF,     // if
  KW_IN,     // in
  KW_ELSE,   // else
  KW_DO,     // do
  KW_AND,    // and
  KW_OR,     // or
  KW_NOT,    // not
  KW_SHL,    // shl
  KW_SHR,    // shr

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
};

struct Token {
  TokenKind kind;
  const char* lexeme;
  usize size;

  // Returns the token lexeme as a proper null-terminated string as opposed to a
  // view.
  String to_string() const;

  // Returns the token in a "dump" format. Primarily used for debugging.
  String get_dump() const;

  // Returns the absolute location of the token.
  AbsLocation location(const FileBuf& source) const;
};

}  // namespace lex

}  // namespace core

}  // namespace via

#endif
