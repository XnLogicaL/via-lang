// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_TOKEN_H_
#define VIA_CORE_TOKEN_H_

#include <via/config.h>
#include <via/util/buffer.h>
#include "location.h"
#include <magic_enum/magic_enum.hpp>

namespace via {

namespace core {

namespace lex {
struct Token;
}

using TokenBuf = Buffer<lex::Token*>;

namespace lex {

struct LexState;

enum TokenKind {
  TK_EOF = 0, // end of file
  TK_ILLEGAL, // unrecognized lexeme

  TK_IDENT, // identifier
  TK_STRING,
  TK_NIL,    // nil
  TK_MIDENT, // macro identifier
  TK_INT,    // integer literal
  TK_BINT,   // binary integer literal
  TK_XINT,   // hexadecimal integer literal
  TK_FP,     // floating point literal
  TK_TRUE,   // true literal
  TK_FALSE,  // false literal

  TK_KW_VAR,   // var
  TK_KW_MACRO, // macro
  TK_KW_FUNC,  // func
  TK_KW_TYPE,  // type
  TK_KW_WHILE, // while
  TK_KW_FOR,   // for
  TK_KW_IF,    // if
  TK_KW_IN,    // in
  TK_KW_ELSE,  // else
  TK_KW_DO,    // do
  TK_KW_AND,   // and
  TK_KW_OR,    // or
  TK_KW_NOT,   // not
  TK_KW_SHL,   // shl
  TK_KW_SHR,   // shr

  TK_DOT,               // .
  TK_COMMA,             // ,
  TK_SEMICOLON,         // ;
  TK_COLON,             // :
  TK_DBCOLON,           // ::
  TK_ARROW,             // ->
  TK_QUESTION,          // ?
  TK_PLUS,              // +
  TK_MINUS,             // -
  TK_ASTERISK,          // *
  TK_FSLASH,            // /
  TK_POW,               // **
  TK_PERCENT,           // %
  TK_AMPERSAND,         // &
  TK_TILDE,             // ~
  TK_CARET,             // ^
  TK_PIPE,              // |
  TK_BANG,              // !
  TK_INC,               // ++
  TK_DEC,               // --
  TK_LESSTHAN,          // <
  TK_GREATERTHAN,       // >
  TK_CONCAT,            // ..
  TK_LPAREN,            // (
  TK_RPAREN,            // )
  TK_LBRACKET,          // [
  TK_RBRACKET,          // ]
  TK_LCURLY,            // {
  TK_RCURLY,            // }
  TK_EQUALS,            // =
  TK_DBEQUALS,          // ==
  TK_PLUSEQUALS,        // +=
  TK_MINUSEQUALS,       // -=
  TK_ASTERISKEQUALS,    // *=
  TK_FSLASHEQUALS,      // /=
  TK_POWEQUALS,         // **=
  TK_PERCENTEQUALS,     // %=
  TK_AMPERSANDEQUALS,   // &=
  TK_CARETEQUALS,       // ^=
  TK_PIPEEQUALS,        // |=
  TK_BANGEQUALS,        // !=
  TK_LESSTHANEQUALS,    // <=
  TK_GREATERTHANEQUALS, // >=
  TK_CONCATEQUALS,      // ..=
};

struct Token {
  TokenKind kind;
  const char* lexeme;
  usize size;

  // Returns the token lexeme as a proper null-terminated string as opposed to a view.
  String to_string() const;

  // Returns the token in a "dump" format. Primarily used for debugging.
  String get_dump() const;

  // Returns the absolute location of the token.
  AbsLocation location(const FileBuf& source) const;
};

} // namespace lex

} // namespace core

} // namespace via

#endif
