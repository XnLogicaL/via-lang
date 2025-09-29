/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "token.h"
#include <format>

via::SourceLoc via::Token::location(const std::string& source) const
{
    const size_t begin = lexeme - source.cbegin().base();
    const size_t end = begin + size;
    return {begin, end};
}

std::string via::Token::get_dump() const
{
    return std::format(
        "[{} '{}']",
        via::to_string(kind),
        (*lexeme == '\0') ? "<eof>" : to_string()
    );
}
