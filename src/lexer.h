// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file lexer.h
 * @brief Declares the Lexer class
 */
#ifndef VIA_HAS_HEADER_LEXER_H
#define VIA_HAS_HEADER_LEXER_H

#include "common.h"
#include "context.h"
#include "token.h"

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @class Lexer
 * @brief Tokenizes the source string found in the translation unit context into a series of tokens.
 */
class Lexer {
public:
  Lexer(Context& lctx)
    : lctx(lctx) {}

  /**
   * @brief Entry point of the class
   */
  void tokenize();

private:
  /**
   * @brief Returns whether if the given character is a valid hexadecimal character
   * @param chr Character
   * @return bool Is hex char
   */
  bool is_hex_char(char chr);

  /**
   * @brief Returns the size of the source string
   * @return size_t Source size
   */
  size_t source_size();

  /**
   * @brief Returns the character the given amount of characters ahead
   * @param ahead Ahead amount
   * @return char Character
   */
  char peek(size_t ahead = 0);

  /**
   * @brief Returns the current character after jumping the given amount of characters ahead
   * @param ahead Ahead amount
   * @return char Character
   */
  char consume(size_t ahead = 1);

  /**
   * @brief Reads a number literal starting from the given position
   * @param pos Position
   * @return Token Number literal as token
   */
  Token read_number(size_t pos);

  /**
   * @brief Reads an identifier starting from the given position
   * @param pos Position
   * @return Token Identifier as token
   */
  Token read_ident(size_t pos);

  /**
   * @brief Reads a string literal starting from the given position
   * @param pos Position
   * @return Token String literal as token
   */
  Token read_string(size_t pos);

  /**
   * @brief Reads and returns the next token in the source string
   * @return Token Next token
   */
  Token get_token();

private:
  size_t pos = 0;    ///< Character cursor
  size_t line = 1;   ///< Line cursor
  size_t offset = 0; ///< Line-based offset cursor

  Context& lctx;
};

/**
 * @brief Returns the given source string as a vector of tokens.
 * @param source Source string
 * @return std::vector<Token> Vector of tokens
 */
inline std::vector<Token> fast_tokenize(std::string source) {
  Context lctx("<unknown>", source);

  Lexer tokenizer(lctx);
  tokenizer.tokenize();

  return lctx.tokens;
}

} // namespace via

/** @} */

#endif
