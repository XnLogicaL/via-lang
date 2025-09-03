// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lexer.h"
#include <fmt/core.h>
#include <cstring>

namespace via
{

// max 3-char symbol lookahead
struct TokenReprPair
{
  const char* str;
  Token::Kind kind;
};

static constexpr TokenReprPair kLexKeywords[] = {
    {"var", Token::Kind::KW_VAR},       {"const", Token::Kind::KW_CONST},
    {"fn", Token::Kind::KW_FN},         {"type", Token::Kind::KW_TYPE},
    {"while", Token::Kind::KW_WHILE},   {"for", Token::Kind::KW_FOR},
    {"if", Token::Kind::KW_IF},         {"in", Token::Kind::KW_IN},
    {"of", Token::Kind::KW_OF},         {"else", Token::Kind::KW_ELSE},
    {"do", Token::Kind::KW_DO},         {"and", Token::Kind::KW_AND},
    {"or", Token::Kind::KW_OR},         {"not", Token::Kind::KW_NOT},
    {"return", Token::Kind::KW_RETURN}, {"as", Token::Kind::KW_AS},
    {"import", Token::Kind::KW_IMPORT}, {"mod", Token::Kind::KW_MODULE},
    {"struct", Token::Kind::KW_STRUCT}, {"enum", Token::Kind::KW_ENUM},
    {"using", Token::Kind::KW_USING},   {"bool", Token::Kind::KW_BOOL},
    {"int", Token::Kind::KW_INT},       {"float", Token::Kind::KW_FLOAT},
    {"string", Token::Kind::KW_STRING},
};

static constexpr TokenReprPair kLexSymbols[] = {
    {".", Token::Kind::PERIOD},
    {",", Token::Kind::COMMA},
    {";", Token::Kind::SEMICOLON},
    {":", Token::Kind::COLON},
    {"::", Token::Kind::COLON_COLON},
    {"->", Token::Kind::ARROW},
    {"?", Token::Kind::QUESTION},
    {"+", Token::Kind::OP_PLUS},
    {"-", Token::Kind::OP_MINUS},
    {"*", Token::Kind::OP_STAR},
    {"/", Token::Kind::OP_SLASH},
    {"**", Token::Kind::OP_STAR_STAR},
    {"%", Token::Kind::OP_PERCENT},
    {"&", Token::Kind::OP_AMP},
    {"~", Token::Kind::OP_TILDE},
    {"^", Token::Kind::OP_CARET},
    {"|", Token::Kind::OP_PIPE},
    {"<<", Token::Kind::OP_SHL},
    {">>", Token::Kind::OP_SHR},
    {"!", Token::Kind::OP_BANG},
    {"++", Token::Kind::OP_PLUS_PLUS},
    {"--", Token::Kind::OP_MINUS_MINUS},
    {"<", Token::Kind::OP_LT},
    {">", Token::Kind::OP_GT},
    {"..", Token::Kind::OP_DOT_DOT},
    {"(", Token::Kind::PAREN_OPEN},
    {")", Token::Kind::PAREN_CLOSE},
    {"[", Token::Kind::BRACKET_OPEN},
    {"]", Token::Kind::BRACKET_CLOSE},
    {"{", Token::Kind::BRACE_OPEN},
    {"}", Token::Kind::BRACE_CLOSE},
    {"=", Token::Kind::OP_EQ},
    {"==", Token::Kind::OP_EQ_EQ},
    {"+=", Token::Kind::OP_PLUS_EQ},
    {"*=", Token::Kind::OP_STAR_EQ},
    {"/=", Token::Kind::OP_SLASH_EQ},
    {"**=", Token::Kind::OP_STAR_STAR_EQ},
    {"%=", Token::Kind::OP_PERCENT_EQ},
    {"&=", Token::Kind::OP_AMP_EQ},
    {"^=", Token::Kind::OP_CARET_EQ},
    {"|=", Token::Kind::OP_PIPE_EQ},
    {"<<=", Token::Kind::OP_SHL_EQ},
    {">>=", Token::Kind::OP_SHR_EQ},
    {"!=", Token::Kind::OP_BANG_EQ},
    {"<=", Token::Kind::OP_LT_EQ},
    {">=", Token::Kind::OP_GT_EQ},
    {"..=", Token::Kind::OP_DOT_DOT_EQ},
};

static consteval usize stringLength(const char* str)
{
  usize len = 0;
  while (str[len] != '\0')
    ++len;

  return len;
}

static consteval usize maxSymbolSize()
{
  usize maxSize = 0;

  for (const auto& sym : kLexSymbols) {
    usize size = stringLength(sym.str);
    if (size > maxSize) {
      maxSize = size;
    }
  }

  return maxSize;
}

static bool isNumeric(Token::Kind* kind, char c)
{
  switch (*kind) {
    case Token::Kind::LIT_INT:
      return isdigit(c) ||
             (c == '.' && *kind != Token::Kind::LIT_FLOAT);  // decimal
    case Token::Kind::LIT_XINT:
      return isxdigit(c);  // hexadecimal
    case Token::Kind::LIT_BINT:
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

char Lexer::advance(int ahead)
{
  char c = *mCursor;
  mCursor += ahead;
  return mCursor < mEnd ? c : '\0';
}

char Lexer::peek(int ahead)
{
  return mCursor + ahead < mEnd ? *(mCursor + ahead) : '\0';
}

Token* Lexer::readNumber()
{
  Token* token = mAlloc.emplace<Token>();
  token->kind = Token::Kind::LIT_INT;
  token->lexeme = mCursor;
  token->size = 0;

  if (peek() == '0') {
    if (peek(1) == 'x')
      token->kind = Token::Kind::LIT_XINT;
    else if (peek(1) == 'b')
      token->kind = Token::Kind::LIT_BINT;
    else
      goto decimal;

    token->size = 2;
    advance(2);  // 0b/0x
  }

decimal:
  char c;
  while ((c = peek()), isNumeric(&token->kind, c)) {
    if (c == '.') {
      if (token->kind == Token::Kind::LIT_INT)
        token->kind = Token::Kind::LIT_FLOAT;
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

Token* Lexer::readString()
{
  Token* token = mAlloc.emplace<Token>();
  token->kind = Token::Kind::LIT_STRING;
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
    token->kind = Token::Kind::ILLEGAL;
  }

  return token;
}

Token* Lexer::readIdentifier()
{
  Token* token = mAlloc.emplace<Token>();
  token->kind = Token::Kind::IDENTIFIER;
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
    token->kind = Token::Kind::LIT_NIL;
  else if (token->size == strlen("true") &&
           strncmp(token->lexeme, "true", token->size) == 0)
    token->kind = Token::Kind::LIT_TRUE;
  else if (token->size == strlen("false") &&
           strncmp(token->lexeme, "false", token->size) == 0)
    token->kind = Token::Kind::LIT_FALSE;

  if (token->kind == Token::Kind::IDENTIFIER && c == '!') {
    token->size++;
    token->kind = Token::Kind::IDENTIFIER_MACRO;
    advance();
  }

  return token;
}

Token* Lexer::readSymbol()
{
  Token* token = mAlloc.emplace<Token>();
  token->lexeme = mCursor;
  token->kind = Token::Kind::ILLEGAL;
  token->size = 1;

  int matchSize = 0;
  auto matchKind = Token::Kind::ILLEGAL;

  char buf[4] = {};

  for (int len = maxSymbolSize(); len >= 1; --len) {
    for (int i = 0; i < len; ++i) {
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
  if (matchKind != Token::Kind::ILLEGAL) {
    token->kind = matchKind;
    token->size = matchSize;

    for (int i = 0; i < matchSize; ++i) {
      advance();
    }
  } else {
    advance();  // advance one char if no match
  }

  return token;
}

bool Lexer::skipComment()
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

TokenTree Lexer::tokenize()
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
  eof->kind = Token::Kind::EOF_;
  eof->lexeme = mCursor;
  eof->size = 0;

  toks.push_back(eof);
  return toks;
}

namespace debug
{

[[nodiscard]] String dump(const TokenTree& tt)
{
  std::ostringstream oss;

  for (const auto* tk : tt) {
    oss << tk->dump() << "\n";
  }

  return oss.str();
}

}  // namespace debug

}  // namespace via
