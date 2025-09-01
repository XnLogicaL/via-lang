// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "token.h"
#include <magic_enum/magic_enum.hpp>

namespace via
{

SourceLoc Token::location(const String& source) const
{
  const usize begin = lexeme - source.cbegin().base();
  const usize end = begin + size;
  return {begin, end};
}

String Token::dump() const
{
  return fmt::format("[{} '{}']", magic_enum::enum_name(kind),
                     (*lexeme == '\0') ? "<eof>" : toString());
}

}  // namespace via
