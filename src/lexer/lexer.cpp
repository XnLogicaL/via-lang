// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lexer.h"

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
  {"in", TK_KW_IN},
  {"else", TK_KW_ELSE},
  {"do", TK_KW_DO},
  {"and", TK_KW_AND},
  {"or", TK_KW_OR},
  {"not", TK_KW_NOT},
  {"shl", TK_KW_SHL},
  {"shr", TK_KW_SHR},
};

static constexpr TokenReprPair SYMBOLS[] = {
  {".", TK_DOT},
  {",", TK_COMMA},
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

  return false;
}

static bool isidentifierinitial(char c) {
  return isalpha(c) || c == '_';
}

static bool isidentifier(char c) {
  return isalnum(c) || c == '_';
}

static char lexer_advance(LexState& L) {
  return *(L.file.cursor++);
}

static char lexer_peek(const LexState& L, int count) {
  return *(L.file.cursor + count);
}

static Token* read_number(LexState& L) {
  Token* token = heap_emplace<Token>(L.al);
  token->kind = TK_INT;
  token->lexeme = L.file.cursor;
  token->size = 0;

  if (lexer_peek(L, 0) == '0') {
    if (lexer_peek(L, 1) == 'x')
      token->kind = TK_XINT;
    else if (lexer_peek(L, 1) == 'b')
      token->kind = TK_BINT;
    else
      goto decimal;

    token->size = 2;
    lexer_advance(L); // 0
    lexer_advance(L); // b/x
  }

decimal:
  char c;
  while ((c = lexer_peek(L, 0)), isnumeric(&token->kind, c)) {
    if (c == '.') {
      if (token->kind == TK_INT)
        token->kind = TK_FP;
      else {
        token->kind = TK_ILLEGAL;
        break;
      }
    }

    lexer_advance(L);
    token->size++;
  }

  return token;
}

static Token* read_string(LexState& L) {
  Token* token = heap_emplace<Token>(L.al);
  token->kind = TK_STRING;
  token->lexeme = L.file.cursor;
  token->size = 1; // for opening quote

  char oq = lexer_advance(L); // opening quote

  char c;
  bool closed = false;
  while ((c = lexer_advance(L)) != '\0') {
    token->size++;
    if (c == oq) {
      closed = true;
      break;
    }
  }

  if (!closed)
    token->kind = TK_ILLEGAL;

  return token;
}

static Token* read_identifier(LexState& L) {
  Token* token = heap_emplace<Token>(L.al);
  token->kind = TK_IDENT;
  token->lexeme = L.file.cursor;
  token->size = 0;

  char c;
  while ((c = lexer_peek(L, 0)), isidentifier(c)) {
    lexer_advance(L);
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
    token->kind = TK_NIL;
  else if (strncmp(token->lexeme, "true", token->size) == 0)
    token->kind = TK_TRUE;
  else if (strncmp(token->lexeme, "false", token->size) == 0)
    token->kind = TK_FALSE;

  if (token->kind == TK_IDENT && c == '!') {
    token->size++;
    token->kind = TK_MIDENT;
    lexer_advance(L);
  }

  return token;
}

static Token* read_symbol(LexState& L) {
  Token* token = heap_emplace<Token>(L.al);
  token->lexeme = L.file.cursor;
  token->kind = TK_ILLEGAL;
  token->size = 1;

  int max_len = 3;
  TokenKind matched_kind = TK_ILLEGAL;
  int matched_size = 0;

  char buf[4] = {};

  for (int len = max_len; len >= 1; --len) {
    for (int i = 0; i < len; ++i)
      buf[i] = L.file.cursor[i];

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
  if (matched_kind != TK_ILLEGAL) {
    token->kind = matched_kind;
    token->size = matched_size;

    for (int i = 0; i < matched_size; ++i)
      lexer_advance(L);
  }
  else
    lexer_advance(L); // advance one char if no match

  return token;
}

static bool skip_comment(LexState& L) {
  if (lexer_peek(L, 0) != '/')
    return false;

  char next = lexer_peek(L, 1);

  if (next == '/') {
    // Line comment: skip until newline or EOF
    lexer_advance(L); // consume first '/'
    lexer_advance(L); // consume second '/'

    while (char c = lexer_peek(L, 0)) {
      if (c == '\n' || c == '\0')
        break;
      lexer_advance(L);
    }

    return true;
  }

  if (next == '*') {
    // Block comment: skip until closing */
    lexer_advance(L); // consume '/'
    lexer_advance(L); // consume '*'

    while (true) {
      char c = lexer_peek(L, 0);
      if (c == '\0')
        break; // EOF without closing */

      if (c == '*' && lexer_peek(L, 1) == '/') {
        lexer_advance(L); // consume '*'
        lexer_advance(L); // consume '/'
        break;
      }

      lexer_advance(L);
    }

    return true;
  }

  return false;
}

TokenBuf lexer_tokenize(LexState& L) {
  Vec<Token*> toks;

  char c;
  while ((c = lexer_peek(L, 0)), c != '\0') {
    if (isspace(c)) {
      lexer_advance(L);
      continue;
    }

    if (skip_comment(L))
      continue;

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

  Token* eof = heap_emplace<Token>(L.al);
  eof->kind = TK_EOF;
  eof->lexeme = L.file.cursor;
  eof->size = 0;

  toks.push_back(eof);

  return TokenBuf(toks.data(), toks.data() + (toks.size() * sizeof(Token*)));
}

void dump_ttree(const TokenBuf& B) {
  for (Token** p = B.data; p < B.data + B.size; p++)
    token_dump(**p);
}

} // namespace via
