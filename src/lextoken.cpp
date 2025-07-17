// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "lextoken.h"
#include "lexstate.h"

namespace via {

const Location abs_location_translate(const FileBuf& buf, usize off) {
  usize line = 0;
  usize line_start = 0;

  for (usize i = 0; i < off; ++i) {
    if (buf.data[i] == '\n') {
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
  String lexeme;

  // check for eof token
  if (strncmp("", T.lexeme, T.size) == 0)
    lexeme = "<eof>";
  else
    lexeme = String(T.lexeme, T.size);

  std::cout << "[" << std::left << std::setfill(' ') << std::setw(12)
            << magic_enum::enum_name(T.kind) << " '" << lexeme << "']\n";
}

} // namespace via
