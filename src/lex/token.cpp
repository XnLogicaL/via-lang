// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "token.h"
#include "common.h"

namespace via {

using enum TokenType;

std::string Token::to_string() const {
  return std::format(
    "Token(type: {}, value: '{}', line: {}, offset: {}, position: {})",
    magic_enum::enum_name(type),
    lexeme,
    line,
    offset,
    position
  );
}

bool Token::is_literal() const {
  return type == LIT_BOOL || type == LIT_FLOAT || type == LIT_INT || type == LIT_STRING;
}

bool Token::is_operator() const {
  return type == OP_ADD || type == OP_DEC || type == OP_DIV || type == OP_EQ || type == OP_EXP
    || type == OP_GEQ || type == OP_GT || type == OP_INC || type == OP_LEQ || type == OP_LT
    || type == OP_MOD || type == OP_MUL || type == OP_NEQ || type == OP_SUB || type == KW_AND
    || type == KW_OR;
}

bool Token::is_modifier() const {
  return type == KW_CONST;
}

int Token::bin_prec() const {
  switch (type) {
  case OP_EXP:
    return 4;
  case OP_MUL:
  case OP_DIV:
  case OP_MOD:
    return 3;
  case OP_ADD:
  case OP_SUB:
    return 2;
  case OP_EQ:
  case OP_NEQ:
  case OP_LT:
  case OP_GT:
  case OP_LEQ:
  case OP_GEQ:
  case KW_AND:
  case KW_OR:
    return 1;
  default:
    return -1;
  }
}

using token_vector = TokenHolder::token_vector;
using at_result = TokenHolder::at_result;

size_t TokenHolder::size() {
  return tokens.size();
}

Token& TokenHolder::at(size_t position) {
  return tokens.at(position);
}

void TokenHolder::push(const Token& Token) {
  tokens.push_back(Token);
}

token_vector& TokenHolder::get() {
  return tokens;
}

} // namespace via
