// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_LEXTOKEN_H
#define VIA_LEXTOKEN_H

#include <common/common.h>
#include <common/heapbuf.h>

namespace via {

struct LexState;

enum TokenKind {
  TK_EOF = 0, // end of file
  TK_ILLEGAL, // unrecognized lexeme

  TK_IDENT, // identifier
  TK_STRING,
  TK_MIDENT, // macro identifier
  TK_INT,    // integer literal
  TK_BINT,   // binary integer literal
  TK_XINT,   // hexadecimal integer literal
  TK_FP,     // floating point literal

  TK_KW_VAR,   // var
  TK_KW_MACRO, // macro
  TK_KW_FUNC,  // func
  TK_KW_TYPE,  // type
  TK_KW_WHILE, // while
  TK_KW_FOR,   // for
  TK_KW_IF,    // if
  TK_KW_ELSE,  // else

  TK_DOT,               // .
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
  TK_AND,               // &&
  TK_OR,                // ||
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

struct Location {
  const size_t begin;
  const size_t end;
};

struct Token {
  TokenKind kind;
  const char* lexeme;
  size_t size;
};

// Buffer of arena allocated token pointers owned by LexState.
using TokenBuf = HeapBuffer<Token*>;

// Dumps token T into standard output.
void token_dump(const Token& T);

// Returns the location of token T.
const Location token_location(const LexState& L, Token& T);

} // namespace via

#endif
