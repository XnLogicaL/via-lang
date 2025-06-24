// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lextoken.h"
#include "lexstate.h"

namespace via {

void token_dump(const Token& T) {
  size_t len = T.size;
  char lexeme[len + 1];
  lexeme[len] = '\0';

  memcpy(&lexeme, T.lexeme, len);

  std::cout << '(' << T.kind << ", " << lexeme << ")\n";
}

Location token_location(LexState* L, Token& token) {
  const size_t pos = token.lexeme - L->file.data;
  return {pos, pos + token.size};
}

} // namespace via
