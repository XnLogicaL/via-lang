/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <string>
#include <via/config.hpp>

namespace via {

class SourceBuffer;

struct SourceLoc
{
    size_t begin;
    size_t end;
};

struct RelSourceLoc
{
    size_t line;
    size_t offset;
};

class SourceBuffer final
{
  public:
    SourceBuffer() = default;
    SourceBuffer(std::string&& source)
        : m_buffer(source)
    {}

    bool is_valid_range(SourceLoc loc) const;
    std::string get_slice(SourceLoc loc) const;
    SourceLoc to_absolute(RelSourceLoc loc) const;
    RelSourceLoc to_relative(SourceLoc loc) const;

    const char* begin() const { return m_buffer.data(); }
    const char* end() const { return m_buffer.data() + m_buffer.size(); }

  private:
    const std::string m_buffer;
};

} // namespace via
