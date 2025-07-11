// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lextoken.h"
#include "lexstate.h"

namespace via {

const Location abs_location_translate(const char* buf, usize off) {
  usize line = 0;
  usize line_start = 0;

  for (usize i = 0; i < off; ++i) {
    if (buf[i] == '\n') {
      ++line;
      line_start = i + 1;
    }
  }

  usize column = off - line_start;
  return {line, column};
}

const AbsLocation token_abs_location(const LexState& L, const Token& T) {
  const usize begin = T.lexeme - L.file.data;
  const usize end = begin + T.size;

  return {begin, end};
}

void token_dump(const Token& T) {
  usize len = T.size;
  char lexeme[len + 1];
  lexeme[len] = '\0';

  std::memcpy(&lexeme, T.lexeme, len);
  std::cout << '(' << T.kind << ", " << lexeme << ")\n";
}

} // namespace via
