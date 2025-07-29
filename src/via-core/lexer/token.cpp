// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "token.h"
#include <cstring>
#include <iomanip>

namespace via {

namespace core {

namespace lex {

AbsLocation Token::location(const FileBuf& source) const {
  const usize begin = lexeme - source.data;
  const usize end = begin + size;

  return {begin, end};
}

String Token::to_string() const {
  return String(lexeme, size);
}

String Token::get_dump() const {
  std::ostringstream oss;
  oss << "[";
  oss << std::left << std::setfill(' ') << std::setw(12) << magic_enum::enum_name(kind);
  oss << " '";

  // check for eof token
  if (strncmp("", lexeme, size) == 0)
    oss << "<eof>";
  else
    oss << to_string();

  oss << "']";
  return oss.str();
}

} // namespace lex

} // namespace core

} // namespace via
