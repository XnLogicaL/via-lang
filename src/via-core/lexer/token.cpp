// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "token.h"

namespace via {

AbsLocation Token::location(const Vec<char>& source) const {
  const usize begin = lexeme - source.cbegin().base();
  const usize end = begin + size;

  return {begin, end};
}

String Token::to_string() const {
  return String(lexeme, size);
}

}  // namespace via
