// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "token.h"
#include "common.h"

namespace via {

using enum token_type;

std::string token::to_string() const {
  return std::format(
    "token(type: {}, value: '{}', line: {}, offset: {}, position: {})",
    magic_enum::enum_name(type),
    lexeme,
    line,
    offset,
    position
  );
}

bool token::is_literal() const {
  return type == LIT_BOOL || type == LIT_FLOAT || type == LIT_INT || type == LIT_STRING;
}

bool token::is_operator() const {
  return type == OP_ADD || type == OP_DECREMENT || type == OP_DIV || type == OP_EQ ||
         type == OP_EXP || type == OP_GEQ || type == OP_GT || type == OP_INCREMENT ||
         type == OP_LEQ || type == OP_LT || type == OP_MOD || type == OP_MUL || type == OP_NEQ ||
         type == OP_SUB || type == KW_AND || type == KW_OR;
}

bool token::is_modifier() const {
  return type == KW_CONST;
}

int token::bin_prec() const {
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

using token_vector = token_stream::token_vector;
using at_result = token_stream::at_result;

size_t token_stream::size() {
  return tokens.size();
}

token& token_stream::at(size_t position) {
  return tokens.at(position);
}

void token_stream::push(const token& token) {
  tokens.push_back(token);
}

token_vector& token_stream::get() {
  return tokens;
}

} // namespace via
