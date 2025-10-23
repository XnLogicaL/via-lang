/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "source.hpp"
#include <limits>
#include "debug.hpp"
#include "lexer/token.hpp"

bool via::SourceBuffer::is_valid_range(SourceLoc loc) const
{
    constexpr auto INVALID = std::numeric_limits<size_t>::max();
    return loc.begin != INVALID && loc.end != INVALID && loc.end >= loc.begin &&
           loc.begin < (m_buffer.size() - 1) && loc.end < (m_buffer.size() - 1);
}

std::string via::SourceBuffer::get_slice(SourceLoc loc) const
{
    debug::require(is_valid_range(loc), "Invalid range");
    std::ostringstream oss;

    for (size_t i = loc.begin; i < loc.end; i++)
        oss << m_buffer.at(i);
    return oss.str();
}

via::SourceLoc via::SourceBuffer::get_location(const char* begin, const char* end) const
{
    size_t lbegin = begin - this->begin();
    return {lbegin, lbegin + (end - begin)};
}

via::SourceLoc via::SourceBuffer::get_location(const Token& token) const
{
    return get_location(token.lexeme, token.lexeme + token.size);
}

via::RelSourceLoc via::SourceBuffer::to_relative(SourceLoc loc) const
{
    size_t init = 0;
    RelSourceLoc rel{0, 0};

    for (size_t i = 0; i < loc.begin; ++i) {
        if (m_buffer.at(i) == '\n') {
            ++rel.line;
            init = i + 1;
        }
    }

    rel.offset = loc.begin - init;
    return rel;
}
