// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lexstate.h"

namespace via {

// max 3-char symbol lookahead
struct TokenReprPair {
  const char* str;
  TokenKind kind;
};

static constexpr TokenReprPair KEYWORDS[] = {
  {"var", TK_KW_VAR},
  {"macro", TK_KW_MACRO},
  {"func", TK_KW_FUNC},
  {"type", TK_KW_TYPE},
  {"while", TK_KW_WHILE},
  {"for", TK_KW_FOR},
  {"if", TK_KW_IF},
  {"else", TK_KW_ELSE},
};

static constexpr TokenReprPair SYMBOLS[] = {
  {".", TK_DOT},
  {";", TK_SEMICOLON},
  {":", TK_COLON},
  {"::", TK_DBCOLON},
  {"->", TK_ARROW},
  {"?", TK_QUESTION},
  {"+", TK_PLUS},
  {"-", TK_MINUS},
  {"*", TK_ASTERISK},
  {"/", TK_FSLASH},
  {"**", TK_POW},
  {"%", TK_PERCENT},
  {"&", TK_AMPERSAND},
  {"~", TK_TILDE},
  {"^", TK_CARET},
  {"|", TK_PIPE},
  {"!", TK_BANG},
  {"++", TK_INC},
  {"--", TK_DEC},
  {"&&", TK_AND},
  {"||", TK_OR},
  {"<", TK_LESSTHAN},
  {">", TK_GREATERTHAN},
  {"..", TK_CONCAT},
  {"(", TK_LPAREN},
  {")", TK_RPAREN},
  {"[", TK_LBRACKET},
  {"]", TK_RBRACKET},
  {"{", TK_LCURLY},
  {"}", TK_RCURLY},
  {"=", TK_EQUALS},
  {"==", TK_DBEQUALS},
  {"+=", TK_PLUSEQUALS},
  {"*=", TK_ASTERISKEQUALS},
  {"/=", TK_FSLASHEQUALS},
  {"**=", TK_POWEQUALS},
  {"%=", TK_PERCENTEQUALS},
  {"&=", TK_AMPERSANDEQUALS},
  {"^=", TK_CARETEQUALS},
  {"|=", TK_PIPEEQUALS},
  {"!=", TK_BANGEQUALS},
  {"<=", TK_LESSTHANEQUALS},
  {">=", TK_GREATERTHANEQUALS},
  {"..=", TK_CONCATEQUALS},
};

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

static bool isidentifierinitial(char c) {
  return isalpha(c) || c == '_';
}

static bool isidentifier(char c) {
  return isalnum(c) || c == '_';
}

static Token* read_number(LexState* L) {
  Token* token = L->ator.emplace<Token>();
  token->kind = TK_INT;
  token->lexeme = L->file.cursor;
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

static Token* read_string(LexState* L) {
  Token* token = L->ator.emplace<Token>();
  token->kind = TK_STRING;
  token->lexeme = L->file.cursor;
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

static Token* read_identifier(LexState* L) {
  Token* token = L->ator.emplace<Token>();
  token->kind = TK_IDENT;
  token->lexeme = L->file.cursor;
  token->size = 0;

  char c;
  while ((c = peek(L, 0)), isidentifier(c)) {
    advance(L);
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

  if (token->kind == TK_IDENT && c == '!') {
    token->size++;
    token->kind = TK_MIDENT;
    advance(L);
  }

  return token;
}

static Token* read_symbol(LexState* L) {
  Token* token = L->ator.emplace<Token>();
  token->lexeme = L->file.cursor;

  // Try to match longest symbol
  char buf[4] = {};
  for (int i = 0; i < 3; ++i) {
    buf[i] = L->file.cursor[i];
    buf[i + 1] = '\0'; // Null terminate

    for (const auto& sym : SYMBOLS) {
      if (strcmp(buf, sym.str) == 0) {
        token->kind = sym.kind;
        token->size = strlen(sym.str);
        for (int j = 0; j < token->size; ++j)
          advance(L);
        return token;
      }
    }
  }

  token->kind = TK_ILLEGAL;
  token->size = 1;
  advance(L);
  return token;
}

char advance(LexState* L) {
  return *(L->file.cursor++);
}

char peek(LexState* L, int count) {
  return *(L->file.cursor + count);
}

TokenBuf tokenize(LexState* L) {
  std::vector<Token*> toks;

  char c;
  while ((c = peek(L, 0)), c != '\0') {
    if (isspace(c)) {
      advance(L);
      continue;
    }

    Token* token;

    if (isdigit(c))
      token = read_number(L);
    else if (isidentifierinitial(c))
      token = read_identifier(L);
    else if (c == '"' || c == '\'')
      token = read_string(L);
    else
      token = read_symbol(L);

    toks.push_back(token);
  }

  Token* eof_ = L->ator.emplace<Token>();
  eof_->kind = TK_EOF;
  eof_->lexeme = L->file.cursor;
  eof_->size = 0;

  toks.push_back(eof_);

  TokenBuf buf(toks.size());
  memcpy(buf.data, toks.data(), toks.size() * sizeof(Token*));
  return buf;
}

void dump_ttree(const TokenBuf& B) {
  for (Token** p = B.data; p < B.data + B.size; p++)
    token_dump(**p);
}

} // namespace via
