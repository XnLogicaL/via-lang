// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "token.h"
#include <cstring>
#include <iomanip>

namespace via {

AbsLocation Token::location(const FileBuf& source) const {
  const usize begin = lexeme - source.data;
  const usize end = begin + size;

  return {begin, end};
}

String Token::to_string() const {
  return String(lexeme, size);
}

}  // namespace via
