// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lexer.h"
#include <fmt/core.h>
#include <cstring>

namespace via {

// max 3-char symbol lookahead
struct TokenReprPair {
  const char* str;
  Token::Kind kind;
};

static constexpr TokenReprPair KEYWORDS[] = {
    {"var", Token::Kind::KW_VAR},     {"macro", Token::Kind::KW_MACRO},
    {"func", Token::Kind::KW_FUNC},   {"type", Token::Kind::KW_TYPE},
    {"while", Token::Kind::KW_WHILE}, {"for", Token::Kind::KW_FOR},
    {"if", Token::Kind::KW_IF},       {"in", Token::Kind::KW_IN},
    {"else", Token::Kind::KW_ELSE},   {"do", Token::Kind::KW_DO},
    {"and", Token::Kind::KW_AND},     {"or", Token::Kind::KW_OR},
    {"not", Token::Kind::KW_NOT},     {"shl", Token::Kind::KW_SHL},
    {"shr", Token::Kind::KW_SHR},
};

static constexpr TokenReprPair SYMBOLS[] = {
    {".", Token::Kind::DOT},
    {",", Token::Kind::COMMA},
    {";", Token::Kind::SEMICOLON},
    {":", Token::Kind::COLON},
    {"::", Token::Kind::DBCOLON},
    {"->", Token::Kind::ARROW},
    {"?", Token::Kind::QUESTION},
    {"+", Token::Kind::PLUS},
    {"-", Token::Kind::MINUS},
    {"*", Token::Kind::ASTERISK},
    {"/", Token::Kind::FSLASH},
    {"**", Token::Kind::POW},
    {"%", Token::Kind::PERCENT},
    {"&", Token::Kind::AMPERSAND},
    {"~", Token::Kind::TILDE},
    {"^", Token::Kind::CARET},
    {"|", Token::Kind::PIPE},
    {"!", Token::Kind::BANG},
    {"++", Token::Kind::INC},
    {"--", Token::Kind::DEC},
    {"<", Token::Kind::LESSTHAN},
    {">", Token::Kind::GREATERTHAN},
    {"..", Token::Kind::CONCAT},
    {"(", Token::Kind::LPAREN},
    {")", Token::Kind::RPAREN},
    {"[", Token::Kind::LBRACKET},
    {"]", Token::Kind::RBRACKET},
    {"{", Token::Kind::LCURLY},
    {"}", Token::Kind::RCURLY},
    {"=", Token::Kind::EQUALS},
    {"==", Token::Kind::DBEQUALS},
    {"+=", Token::Kind::PLUSEQUALS},
    {"*=", Token::Kind::ASTERISKEQUALS},
    {"/=", Token::Kind::FSLASHEQUALS},
    {"**=", Token::Kind::POWEQUALS},
    {"%=", Token::Kind::PERCENTEQUALS},
    {"&=", Token::Kind::AMPERSANDEQUALS},
    {"^=", Token::Kind::CARETEQUALS},
    {"|=", Token::Kind::PIPEEQUALS},
    {"!=", Token::Kind::BANGEQUALS},
    {"<=", Token::Kind::LESSTHANEQUALS},
    {">=", Token::Kind::GREATERTHANEQUALS},
    {"..=", Token::Kind::CONCATEQUALS},
};

static bool is_numeric(Token::Kind* kind, char c) {
  switch (*kind) {
      // clang-format off
  case Token::Kind::INT:  return isdigit(c) || (c == '.' && *kind != Token::Kind::FP); // decimal
  case Token::Kind::XINT: return isxdigit(c); // hexadecimal
  case Token::Kind::BINT: return c == '0' || c == '1'; // binary
    // clang-format on
    default:
      break;
  }

  return false;
}

static bool is_identifier_initial(char c) {
  return isalpha(c) || c == '_';
}

static bool is_identifier(char c) {
  return isalnum(c) || c == '_';
}

static bool is_string_delimiter(char c) {
  return c == '"' || c == '\'' || c == '`';
}

char Lexer::advance(int ahead) {
  return *(cursor += ahead);
}

char Lexer::peek(int ahead) {
  return *(cursor + ahead);
}

Token* Lexer::read_number() {
  Token* token = alloc.emplace<Token>();
  token->kind = Token::Kind::INT;
  token->lexeme = cursor;
  token->size = 0;

  if (peek() == '0') {
    if (peek(1) == 'x')
      token->kind = Token::Kind::XINT;
    else if (peek(1) == 'b')
      token->kind = Token::Kind::BINT;
    else
      goto decimal;

    token->size = 2;
    advance();  // 0
    advance();  // b/x
  }

decimal:
  char c;
  while ((c = peek()), is_numeric(&token->kind, c)) {
    if (c == '.') {
      if (token->kind == Token::Kind::INT)
        token->kind = Token::Kind::FP;
      else {
        token->kind = Token::Kind::ILLEGAL;
        break;
      }
    }

    advance();
    token->size++;
  }

  return token;
}

Token* Lexer::read_string() {
  Token* token = alloc.emplace<Token>();
  token->kind = Token::Kind::STRING;
  token->lexeme = cursor;
  token->size = 1;  // for opening quote

  char del = advance();  // opening quote

  char c;
  bool closed = false;
  while ((c = advance()) != '\0') {
    token->size++;
    if (c == del) {
      closed = true;
      break;
    }
  }

  if (!closed)
    token->kind = Token::Kind::ILLEGAL;

  return token;
}

Token* Lexer::read_identifier() {
  Token* token = alloc.emplace<Token>();
  token->kind = Token::Kind::IDENT;
  token->lexeme = cursor;
  token->size = 0;

  char c;
  while ((c = peek()), is_identifier(c)) {
    advance();
    token->size++;
  }

  for (const auto& kw : KEYWORDS) {
    if (strlen(kw.str) != token->size)
      continue;

    if (strncmp(kw.str, token->lexeme, token->size) == 0) {
      token->kind = kw.kind;
      break;
    }
  }

  if (strncmp(token->lexeme, "nil", token->size) == 0)
    token->kind = Token::Kind::NIL;
  else if (strncmp(token->lexeme, "true", token->size) == 0)
    token->kind = Token::Kind::TRUE;
  else if (strncmp(token->lexeme, "false", token->size) == 0)
    token->kind = Token::Kind::FALSE;

  if (token->kind == Token::Kind::IDENT && c == '!') {
    token->size++;
    token->kind = Token::Kind::MIDENT;
    advance();
  }

  return token;
}

Token* Lexer::read_symbol() {
  Token* token = alloc.emplace<Token>();
  token->lexeme = cursor;
  token->kind = Token::Kind::ILLEGAL;
  token->size = 1;

  int max_len = 3;
  Token::Kind matched_kind = Token::Kind::ILLEGAL;
  int matched_size = 0;

  char buf[4] = {};

  for (int len = max_len; len >= 1; --len) {
    for (int i = 0; i < len; ++i)
      buf[i] = cursor[i];

    buf[len] = '\0';

    for (const auto& sym : SYMBOLS) {
      if (strcmp(buf, sym.str) == 0) {
        matched_kind = sym.kind;
        matched_size = len;
        goto found;
      }
    }
  }

found:
  if (matched_kind != Token::Kind::ILLEGAL) {
    token->kind = matched_kind;
    token->size = matched_size;

    for (int i = 0; i < matched_size; ++i)
      advance();
  } else
    advance();  // advance one char if no match

  return token;
}

bool Lexer::skip_comment() {
  if (peek() != '/')
    return false;

  char next = peek(1);

  if (next == '/') {
    // Line comment: skip until newline or EOF
    advance(2);  // consume first '//'

    while (char c = peek()) {
      if (c == '\n' || c == '\0')
        break;
      advance(2);
    }

    return true;
  }

  if (next == '*') {
    // Block comment: skip until closing */
    advance(2);  // consume '/*'

    while (true) {
      char c = peek();
      if (c == '\0')
        break;  // EOF without closing */

      if (c == '*' && peek(1) == '/') {
        advance(2);  // consume '*/'
        break;
      }

      advance();
    }

    return true;
  }

  return false;
}

TokenBuf Lexer::tokenize() {
  Vec<Token*> toks;

  char c;
  while ((c = peek()), c != '\0') {
    if (isspace(c)) {
      advance();
      continue;
    }

    if (skip_comment())
      continue;

    Token* token;

    if (isdigit(c))
      token = read_number();
    else if (is_identifier_initial(c))
      token = read_identifier();
    else if (is_string_delimiter(c))
      token = read_string();
    else
      token = read_symbol();

    toks.push_back(token);
  }

  Token* eof = alloc.emplace<Token>();
  eof->kind = Token::Kind::EOF_;
  eof->lexeme = cursor;
  eof->size = 0;

  toks.push_back(eof);
  return TokenBuf(toks.data(), toks.data() + (toks.size() * sizeof(Token*)));
}

}  // namespace via
