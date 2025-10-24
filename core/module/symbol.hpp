/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstdint>
#include <deque>
#include <sstream>
#include <string>
#include "support/intern.hpp"

namespace via {

using SymbolId = uint64_t;
using QualName = std::deque<std::string>;

inline std::string to_string(const QualName& path)
{
    std::ostringstream oss;
    for (size_t i = 0; i < path.size(); i++) {
        if (i > 0) {
            oss << "::";
        }
        oss << path[i];
    }
    return oss.str();
}

class SymbolTable final: public InternTable<std::string, SymbolId>
{
  public:
    using InternTable::intern;

    SymbolId intern(const QualName& path) { return intern(via::to_string(path)); }
    std::string to_string() const noexcept;
};

} // namespace via
