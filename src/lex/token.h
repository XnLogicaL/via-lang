// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_TOKEN_H
#define VIA_HAS_HEADER_TOKEN_H

#include "common.h"

#include <arena.h>

namespace via {

enum class TokenType {
  // Keywords
  KW_DO,       // do
  KW_IN,       // in
  KW_LOCAL,    // let
  KW_GLOBAL,   // glb
  KW_AS,       // as
  KW_IF,       // if
  KW_ELSE,     // else
  KW_ELIF,     // elif
  KW_WHILE,    // while
  KW_FOR,      // for
  KW_RETURN,   // return
  KW_FUNC,     // fn
  KW_CONST,    // const
  KW_NEW,      // new
  KW_BREAK,    // break
  KW_CONTINUE, // continue
  KW_MATCH,    // switch
  KW_CASE,     // case
  KW_DEFAULT,  // default
  KW_AND,      // and
  KW_NOT,      // not
  KW_OR,       // or
  KW_STRUCT,   // struct
  KW_IMPORT,   // import
  KW_EXPORT,   // export
  KW_MACRO,    // macro
  KW_DEFINE,   // define
  KW_TYPE,     // type
  KW_TRAIT,    // trait
  KW_DEFINED,  // defined
  KW_PRAGMA,   // pragma
  KW_ENUM,     // enum
  KW_TRY,      // try
  KW_CATCH,    // catch
  KW_RAISE,    // raise
  KW_AUTO,     // auto
  KW_DEFER,    // defer
  KW_TYPEOF,
  KW_NAMEOF,

  // Operators
  OP_ADD, // +
  OP_SUB, // -
  OP_MUL, // *
  OP_DIV, // /
  OP_EXP, // ^
  OP_MOD, // %
  OP_EQ,  // ==
  OP_NEQ, // !=
  OP_LT,  // <
  OP_GT,  // >
  OP_LEQ, // <=
  OP_GEQ, // >=
  OP_INC, // ++
  OP_DEC, // --
  OP_LEN,
  // Pseudo-operators
  EQ,      // =
  RETURNS, // ->

  // Literals
  LIT_INT,    // Integer literals
  LIT_FLOAT,  // Floating-point literals
  LIT_HEX,    // Hexadecimal number literals
  LIT_BINARY, // Binary number literals
  LIT_STRING, // String literals
  LIT_BOOL,   // Boolean literals
  LIT_NIL,    // Nil literal (just "Nil" lol)

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
  UNKNOWN // Unknown Token
};

using token_lexeme_t = std::string;

struct Token {
  TokenType type = TokenType::UNKNOWN;
  token_lexeme_t lexeme = "";
  size_t line, offset, position;

  explicit Token() = default;
  explicit Token(TokenType type, std::string lexeme, size_t line, size_t offset, size_t position)
    : type(type),
      lexeme(lexeme),
      line(line),
      offset(offset),
      position(position) {}

  // Returns the token as a string.
  std::string to_string() const;

  // Returns whether if the tokens lexeme is a value literal.
  bool is_literal() const;

  // Returns whether if the tokens lexeme is an operator.
  bool is_operator() const;

  // Returns whether if the tokens lexeme is a modifier.
  bool is_modifier() const;

  // Returns the operator precedence of the tokens lexeme or -1 if impossible.
  int bin_prec() const;
};

} // namespace via

#endif
