// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lexer.h"
#include <fmt/core.h>
#include <cstring>

using enum via::Token::Kind;

// max 3-char symbol lookahead
struct TokenReprPair
{
  const char* str;
  via::Token::Kind kind;
};

static constexpr TokenReprPair kLexKeywords[] = {
  {"var", KW_VAR},       {"const", KW_CONST},   {"fn", KW_FN},
  {"type", KW_TYPE},     {"while", KW_WHILE},   {"for", KW_FOR},
  {"if", KW_IF},         {"in", KW_IN},         {"of", KW_OF},
  {"else", KW_ELSE},     {"do", KW_DO},         {"and", KW_AND},
  {"or", KW_OR},         {"not", KW_NOT},       {"return", KW_RETURN},
  {"as", KW_AS},         {"import", KW_IMPORT}, {"mod", KW_MODULE},
  {"struct", KW_STRUCT}, {"enum", KW_ENUM},     {"using", KW_USING},
  {"bool", KW_BOOL},     {"isize", KW_INT},     {"float", KW_FLOAT},
  {"string", KW_STRING},
};

static constexpr TokenReprPair kLexSymbols[] = {
  {".", PERIOD},
  {",", COMMA},
  {";", SEMICOLON},
  {":", COLON},
  {"::", COLON_COLON},
  {"->", ARROW},
  {"?", QUESTION},
  {"+", OP_PLUS},
  {"-", OP_MINUS},
  {"*", OP_STAR},
  {"/", OP_SLASH},
  {"**", OP_STAR_STAR},
  {"%", OP_PERCENT},
  {"&", OP_AMP},
  {"~", OP_TILDE},
  {"^", OP_CARET},
  {"|", OP_PIPE},
  {"<<", OP_SHL},
  {">>", OP_SHR},
  {"!", OP_BANG},
  {"++", OP_PLUS_PLUS},
  {"--", OP_MINUS_MINUS},
  {"<", OP_LT},
  {">", OP_GT},
  {"..", OP_DOT_DOT},
  {"(", PAREN_OPEN},
  {")", PAREN_CLOSE},
  {"[", BRACKET_OPEN},
  {"]", BRACKET_CLOSE},
  {"{", BRACE_OPEN},
  {"}", BRACE_CLOSE},
  {"=", OP_EQ},
  {"==", OP_EQ_EQ},
  {"+=", OP_PLUS_EQ},
  {"*=", OP_STAR_EQ},
  {"/=", OP_SLASH_EQ},
  {"**=", OP_STAR_STAR_EQ},
  {"%=", OP_PERCENT_EQ},
  {"&=", OP_AMP_EQ},
  {"^=", OP_CARET_EQ},
  {"|=", OP_PIPE_EQ},
  {"<<=", OP_SHL_EQ},
  {">>=", OP_SHR_EQ},
  {"!=", OP_BANG_EQ},
  {"<=", OP_LT_EQ},
  {">=", OP_GT_EQ},
  {"..=", OP_DOT_DOT_EQ},
};

static consteval via::usize stringLength(const char* str)
{
  via::usize len = 0;
  while (str[len] != '\0') {
    ++len;
  }

  return len;
}

static consteval via::usize maxSymbolSize()
{
  via::usize maxSize = 0;

  for (const auto& sym : kLexSymbols) {
    via::usize size = stringLength(sym.str);
    if (size > maxSize) {
      maxSize = size;
    }
  }

  return maxSize;
}

static bool isNumeric(via::Token::Kind* kind, char c)
{
  switch (*kind) {
    case LIT_INT:
      return isdigit(c) || (c == '.' && *kind != LIT_FLOAT);  // decimal
    case LIT_XINT:
      return isxdigit(c);  // hexadecimal
    case LIT_BINT:
      return c == '0' || c == '1';  // binary
    default:
      break;
  }

  return false;
}

static bool isIdentifierInitial(char c)
{
  return isalpha(c) || c == '_';
}

static bool isIdentifier(char c)
{
  return isalnum(c) || c == '_';
}

static bool isStringDelimiter(char c)
{
  return c == '"' || c == '\'' || c == '`';
}

char via::Lexer::advance(isize ahead)
{
  char c = *mCursor;
  mCursor += ahead;
  return mCursor < mEnd ? c : '\0';
}

char via::Lexer::peek(isize ahead)
{
  return mCursor + ahead < mEnd ? *(mCursor + ahead) : '\0';
}

via::Token* via::Lexer::readNumber()
{
  Token* token = mAlloc.emplace<Token>();
  token->kind = LIT_INT;
  token->lexeme = mCursor;
  token->size = 0;

  if (peek() == '0') {
    if (peek(1) == 'x')
      token->kind = LIT_XINT;
    else if (peek(1) == 'b')
      token->kind = LIT_BINT;
    else
      goto decimal;

    token->size = 2;
    advance(2);  // 0b/0x
  }

decimal:
  char c;
  while ((c = peek()), isNumeric(&token->kind, c)) {
    if (c == '.') {
      if (token->kind == LIT_INT)
        token->kind = LIT_FLOAT;
      else {
        token->kind = ILLEGAL;
        break;
      }
    }

    advance();
    token->size++;
  }

  return token;
}

via::Token* via::Lexer::readString()
{
  Token* token = mAlloc.emplace<Token>();
  token->kind = LIT_STRING;
  token->lexeme = mCursor;

  char del = advance();
  token->size = 1;

  char c;
  bool closed = false;
  while ((c = advance()) != '\0') {
    token->size++;

    if (c == '\\') {
      if (peek() != '\0') {
        advance();
        token->size++;
      }
    } else if (c == del) {
      closed = true;
      break;
    }
  }

  if (!closed) {
    token->size = 1;
    token->kind = ILLEGAL;
  }

  return token;
}

via::Token* via::Lexer::readIdentifier()
{
  Token* token = mAlloc.emplace<Token>();
  token->kind = IDENTIFIER;
  token->lexeme = mCursor;
  token->size = 0;

  char c;
  while ((c = peek()), isIdentifier(c)) {
    advance();
    token->size++;
  }

  for (const auto& kw : kLexKeywords) {
    if (strlen(kw.str) != token->size)
      continue;

    if (strncmp(kw.str, token->lexeme, token->size) == 0) {
      token->kind = kw.kind;
      break;
    }
  }

  if (token->size == strlen("nil") &&
      strncmp(token->lexeme, "nil", token->size) == 0)
    token->kind = LIT_NIL;
  else if (token->size == strlen("true") &&
           strncmp(token->lexeme, "true", token->size) == 0)
    token->kind = LIT_TRUE;
  else if (token->size == strlen("false") &&
           strncmp(token->lexeme, "false", token->size) == 0)
    token->kind = LIT_FALSE;

  if (token->kind == IDENTIFIER && c == '!') {
    token->size++;
    token->kind = IDENTIFIER_MACRO;
    advance();
  }

  return token;
}

via::Token* via::Lexer::readSymbol()
{
  Token* token = mAlloc.emplace<Token>();
  token->lexeme = mCursor;
  token->kind = ILLEGAL;
  token->size = 1;

  isize matchSize = 0;
  auto matchKind = ILLEGAL;

  char buf[4] = {};

  for (isize len = maxSymbolSize(); len >= 1; --len) {
    for (isize i = 0; i < len; ++i) {
      buf[i] = mCursor[i];
    }

    buf[len] = '\0';

    for (const auto& sym : kLexSymbols) {
      if (len == strlen(sym.str) && strcmp(buf, sym.str) == 0) {
        matchKind = sym.kind;
        matchSize = len;
        goto found;
      }
    }
  }

found:
  if (matchKind != ILLEGAL) {
    token->kind = matchKind;
    token->size = matchSize;

    for (isize i = 0; i < matchSize; ++i) {
      advance();
    }
  } else {
    advance();  // advance one char if no match
  }

  return token;
}

bool via::Lexer::skipComment()
{
  if (peek() != '/') {
    return false;
  }

  char next = peek(1);
  if (next == '/') {
    advance(2);  // consume first '//'

    while (char c = peek()) {
      if (c == '\n' || c == '\0')
        break;

      advance();
    }

    return true;
  }

  if (next == '*') {
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

via::TokenTree via::Lexer::tokenize()
{
  TokenTree toks;

  char c;
  while ((c = peek()), c != '\0') {
    if (isspace(c)) {
      advance();
      continue;
    }

    if (skipComment())
      continue;

    Token* token;

    if (std::isdigit(c))
      token = readNumber();
    else if (isIdentifierInitial(c))
      token = readIdentifier();
    else if (isStringDelimiter(c))
      token = readString();
    else
      token = readSymbol();

    toks.push_back(token);
  }

  Token* eof = mAlloc.emplace<Token>();
  eof->kind = EOF_;
  eof->lexeme = mCursor;
  eof->size = 0;

  toks.push_back(eof);
  return toks;
}

[[nodiscard]] std::string dump(const via::TokenTree& tt)
{
  std::ostringstream oss;
  for (const auto* tk : tt) {
    oss << tk->dump() << "\n";
  }

  return oss.str();
}
