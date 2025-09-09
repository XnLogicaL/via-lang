/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "token.h"
#include <magic_enum/magic_enum.hpp>

via::SourceLoc via::Token::location(const std::string& source) const
{
  const usize begin = lexeme - source.cbegin().base();
  const usize end = begin + size;
  return {begin, end};
}

std::string via::Token::dump() const
{
  return std::format("[{} '{}']", magic_enum::enum_name(kind),
                     (*lexeme == '\0') ? "<eof>" : toString());
}
