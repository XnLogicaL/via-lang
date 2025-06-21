// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lexstate.h"

namespace via {

namespace lex {

// why does the C standard library not have this??
static bool isbdigit(char c) {
  return c == '0' || c == '1';
}

static bool isnumeric(TokenKind* kind, char c) {
  switch (*kind) {
  // clang-format off
  case TK_INT:  return isdigit(c) || (c == '.' && *kind != TK_FP); // decimal
  case TK_XINT: return isxdigit(c); // hexadecimal
  case TK_BINT: return isbdigit(c); // binary
  // clang-format on
  default:
    break;
  }

  std::unreachable();
}

static Token* read_number(State* L) {
  Token* token = L->tok_ator.emplace<Token>();
  token->kind = TK_INT;
  token->lexeme = L->cursor;
  token->size = 0;

  if (peek(L, 0) == '0') {
    if (peek(L, 1) == 'x')
      token->kind = TK_XINT;
    else if (peek(L, 1) == 'b')
      token->kind = TK_BINT;
    else
      goto decimal;

    advance(L); // 0
    advance(L); // b/x
  decimal:
  }

  char c;
  while ((c = advance(L)), isnumeric(&token->kind, c)) {
    if (c == '.')
      token->kind = TK_FP;
    token->size++;
  }

  return token;
}

static Token* read_string(State* L) {
  Token* token = L->tok_ator.emplace<Token>();
  token->kind = TK_STRING;
  token->lexeme = L->cursor;
  token->size = 1; // for opening quote

  char oq = advance(L); // opening quote

  char c;
  bool closed = false;
  while ((c = advance(L)) != '\0') {
    token->size++;
    if (c == oq) {
      closed = true;
      break;
    }
  }

  if (!closed)
    token->kind = TK_ILLEGAL;
  else
    token->size++; // include closing quote

  return token;
}

static Token* read_identifier(State* L) {
  Token* token = L->tok_ator.emplace<Token>();
  token->kind = TK_IDENT;
  token->lexeme = L->cursor;
  token->size = 0;

  auto is_legal = [L](char c) constexpr->bool {
    return isalnum(c) || c == '_';
  };

  char c;
  while ((c = advance(L)), is_legal(c))
    token->size++;

  if (c == '!') {
    token->size++;
    token->kind = TK_MIDENT;
    advance(L);
  }

  return token;
}

char advance(State* L) {
  return *(L->cursor++);
}

char peek(State* L, int count) {
  return *(L->cursor + count);
}

TokenBuf lex(State* L) {
  std::vector<Token*> toks;

  char c;
  while ((c = peek(L, 0)), c != '\0') {
    Token* token;

    if (isdigit(c))
      token = read_number(L);
    else if (isalnum(c))
      token = read_identifier(L);
    else if (c == '"' || c == '\'')
      token = read_string(L);

    if (!token) {
    }

    toks.push_back(token);
  }

  Token* eof_ = L->tok_ator.emplace<Token>();
  eof_->kind = TK_EOF;
  eof_->lexeme = L->cursor;
  eof_->size = 0;

  toks.push_back(eof_);

  TokenBuf buf(toks.size());
  memcpy(buf.data, toks.data(), toks.size());
  return buf;
}

} // namespace lex

} // namespace via
