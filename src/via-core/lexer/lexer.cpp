// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lexer.h"
#include <fmt/core.h>
#include <cstring>

namespace via {

namespace core {

namespace lex {

// max 3-char symbol lookahead
struct TokenReprPair {
  const char* str;
  TokenKind kind;
};

static constexpr TokenReprPair KEYWORDS[] = {
    {"var", TokenKind::KW_VAR},     {"macro", TokenKind::KW_MACRO},
    {"func", TokenKind::KW_FUNC},   {"type", TokenKind::KW_TYPE},
    {"while", TokenKind::KW_WHILE}, {"for", TokenKind::KW_FOR},
    {"if", TokenKind::KW_IF},       {"in", TokenKind::KW_IN},
    {"else", TokenKind::KW_ELSE},   {"do", TokenKind::KW_DO},
    {"and", TokenKind::KW_AND},     {"or", TokenKind::KW_OR},
    {"not", TokenKind::KW_NOT},     {"shl", TokenKind::KW_SHL},
    {"shr", TokenKind::KW_SHR},
};

static constexpr TokenReprPair SYMBOLS[] = {
    {".", TokenKind::DOT},
    {",", TokenKind::COMMA},
    {";", TokenKind::SEMICOLON},
    {":", TokenKind::COLON},
    {"::", TokenKind::DBCOLON},
    {"->", TokenKind::ARROW},
    {"?", TokenKind::QUESTION},
    {"+", TokenKind::PLUS},
    {"-", TokenKind::MINUS},
    {"*", TokenKind::ASTERISK},
    {"/", TokenKind::FSLASH},
    {"**", TokenKind::POW},
    {"%", TokenKind::PERCENT},
    {"&", TokenKind::AMPERSAND},
    {"~", TokenKind::TILDE},
    {"^", TokenKind::CARET},
    {"|", TokenKind::PIPE},
    {"!", TokenKind::BANG},
    {"++", TokenKind::INC},
    {"--", TokenKind::DEC},
    {"<", TokenKind::LESSTHAN},
    {">", TokenKind::GREATERTHAN},
    {"..", TokenKind::CONCAT},
    {"(", TokenKind::LPAREN},
    {")", TokenKind::RPAREN},
    {"[", TokenKind::LBRACKET},
    {"]", TokenKind::RBRACKET},
    {"{", TokenKind::LCURLY},
    {"}", TokenKind::RCURLY},
    {"=", TokenKind::EQUALS},
    {"==", TokenKind::DBEQUALS},
    {"+=", TokenKind::PLUSEQUALS},
    {"*=", TokenKind::ASTERISKEQUALS},
    {"/=", TokenKind::FSLASHEQUALS},
    {"**=", TokenKind::POWEQUALS},
    {"%=", TokenKind::PERCENTEQUALS},
    {"&=", TokenKind::AMPERSANDEQUALS},
    {"^=", TokenKind::CARETEQUALS},
    {"|=", TokenKind::PIPEEQUALS},
    {"!=", TokenKind::BANGEQUALS},
    {"<=", TokenKind::LESSTHANEQUALS},
    {">=", TokenKind::GREATERTHANEQUALS},
    {"..=", TokenKind::CONCATEQUALS},
};

// why does the C standard library not have this??
static bool isbdigit(char c) {
  return c == '0' || c == '1';
}

static bool isnumeric(TokenKind* kind, char c) {
  switch (*kind) {
      // clang-format off
  case TokenKind::INT:  return isdigit(c) || (c == '.' && *kind != TokenKind::FP); // decimal
  case TokenKind::XINT: return isxdigit(c); // hexadecimal
  case TokenKind::BINT: return isbdigit(c); // binary
    // clang-format on
    default:
      break;
  }

  return false;
}

static bool isidentifierinitial(char c) {
  return isalpha(c) || c == '_';
}

static bool isidentifier(char c) {
  return isalnum(c) || c == '_';
}

char Lexer::advance() {
  return *(file.cursor++);
}

char Lexer::peek(int count) {
  return *(file.cursor + count);
}

Token* Lexer::read_number() {
  Token* token = alloc.emplace<Token>();
  token->kind = TokenKind::INT;
  token->lexeme = file.cursor;
  token->size = 0;

  if (peek(0) == '0') {
    if (peek(1) == 'x')
      token->kind = TokenKind::XINT;
    else if (peek(1) == 'b')
      token->kind = TokenKind::BINT;
    else
      goto decimal;

    token->size = 2;
    advance();  // 0
    advance();  // b/x
  }

decimal:
  char c;
  while ((c = peek(0)), isnumeric(&token->kind, c)) {
    if (c == '.') {
      if (token->kind == TokenKind::INT)
        token->kind = TokenKind::FP;
      else {
        token->kind = TokenKind::ILLEGAL;
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
  token->kind = TokenKind::STRING;
  token->lexeme = file.cursor;
  token->size = 1;  // for opening quote

  char oq = advance();  // opening quote

  char c;
  bool closed = false;
  while ((c = advance()) != '\0') {
    token->size++;
    if (c == oq) {
      closed = true;
      break;
    }
  }

  if (!closed)
    token->kind = TokenKind::ILLEGAL;

  return token;
}

Token* Lexer::read_identifier() {
  Token* token = alloc.emplace<Token>();
  token->kind = TokenKind::IDENT;
  token->lexeme = file.cursor;
  token->size = 0;

  char c;
  while ((c = peek(0)), isidentifier(c)) {
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
    token->kind = TokenKind::NIL;
  else if (strncmp(token->lexeme, "true", token->size) == 0)
    token->kind = TokenKind::TRUE;
  else if (strncmp(token->lexeme, "false", token->size) == 0)
    token->kind = TokenKind::FALSE;

  if (token->kind == TokenKind::IDENT && c == '!') {
    token->size++;
    token->kind = TokenKind::MIDENT;
    advance();
  }

  return token;
}

Token* Lexer::read_symbol() {
  Token* token = alloc.emplace<Token>();
  token->lexeme = file.cursor;
  token->kind = TokenKind::ILLEGAL;
  token->size = 1;

  int max_len = 3;
  TokenKind matched_kind = TokenKind::ILLEGAL;
  int matched_size = 0;

  char buf[4] = {};

  for (int len = max_len; len >= 1; --len) {
    for (int i = 0; i < len; ++i)
      buf[i] = file.cursor[i];

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
  if (matched_kind != TokenKind::ILLEGAL) {
    token->kind = matched_kind;
    token->size = matched_size;

    for (int i = 0; i < matched_size; ++i)
      advance();
  } else
    advance();  // advance one char if no match

  return token;
}

bool Lexer::skip_comment() {
  if (peek(0) != '/')
    return false;

  char next = peek(1);

  if (next == '/') {
    // Line comment: skip until newline or EOF
    advance();  // consume first '/'
    advance();  // consume second '/'

    while (char c = peek(0)) {
      if (c == '\n' || c == '\0')
        break;
      advance();
    }

    return true;
  }

  if (next == '*') {
    // Block comment: skip until closing */
    advance();  // consume '/'
    advance();  // consume '*'

    while (true) {
      char c = peek(0);
      if (c == '\0')
        break;  // EOF without closing */

      if (c == '*' && peek(1) == '/') {
        advance();  // consume '*'
        advance();  // consume '/'
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
  while ((c = peek(0)), c != '\0') {
    if (isspace(c)) {
      advance();
      continue;
    }

    if (skip_comment())
      continue;

    Token* token;

    if (isdigit(c))
      token = read_number();
    else if (isidentifierinitial(c))
      token = read_identifier();
    else if (c == '"' || c == '\'')
      token = read_string();
    else
      token = read_symbol();

    toks.push_back(token);
  }

  Token* eof = alloc.emplace<Token>();
  eof->kind = TokenKind::EOF_;
  eof->lexeme = file.cursor;
  eof->size = 0;

  toks.push_back(eof);

  return TokenBuf(toks.data(), toks.data() + (toks.size() * sizeof(Token*)));
}

void dump_ttree(const TokenBuf& B) {
  for (Token** p = B.data; p < B.data + B.size; p++)
    fmt::println("{}", (*p)->get_dump());
}

}  // namespace lex

}  // namespace core

}  // namespace via
