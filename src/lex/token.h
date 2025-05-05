// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file token.h
 * @brief Declares lex related token structures.
 */
#ifndef VIA_HAS_HEADER_TOKEN_H
#define VIA_HAS_HEADER_TOKEN_H

#include "common.h"

#include <arena.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @enum TokenType
 * @brief Describes the type of a token's lexeme.
 */
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
  KW_ERROR,    // error
  KW_PRINT,    // print
  KW_AUTO,     // auto
  KW_DEFER,    // defer
  KW_TYPEOF,   // typeof
  KW_NAMEOF,   // nameof

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

/**
 * @struct Token
 * @brief Token structure that contains a lexeme and location.
 */
struct Token {
  TokenType type = TokenType::UNKNOWN;       ///< Type of the token's lexeme
  std::string lexeme;                        ///< The token's lexeme
  size_t line = 0, offset = 0, position = 0; ///< Location of the token

  explicit Token() = default;
  explicit Token(TokenType type, std::string lexeme, size_t line, size_t offset, size_t position)
    : type(type),
      lexeme(lexeme),
      line(line),
      offset(offset),
      position(position) {}

  /**
   * @brief Returns the token represented as an std::string.
   * @return std::string Token string
   */
  std::string to_string() const;

  /**
   * @brief Returns whether if the token's lexeme is classified as a literal.
   * @return bool Is literal
   */
  bool is_literal() const;

  /**
   * @brief Returns whether if the token's lexeme is classified as an operator.
   * @return bool Is operator
   */
  bool is_operator() const;

  /**
   * @brief Returns whether if the token's lexeme is classified as a modifier keyword.
   * @return bool Is modifier keyword
   */
  bool is_modifier() const;

  /**
   * @brief Returns the binary precedence of the token if it is an operator.
   * @return int Operator precedence or -1 if not operator
   */
  int bin_prec() const;
};

} // namespace via

/** @} */

#endif
