/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "symbol.hpp"
#include "support/ansi.hpp"

std::string via::SymbolTable::to_string() const noexcept
{
    std::ostringstream oss;
    oss << "(global) "
        << ansi::format(
               "[disassembly of symbol table]:\n",
               ansi::Foreground::YELLOW,
               ansi::Background::NONE,
               ansi::Style::UNDERLINE
           );

    oss << ansi::format(
        "   id      symbol     \n"
        "  [----]  [----------]\n",
        ansi::Foreground::NONE,
        ansi::Background::NONE,
        ansi::Style::FAINT
    );

    for (const auto& symbol: m_reverse) {
        oss << "   "
            << ansi::format(
                   std::format("{:0>4}", symbol.first),
                   ansi::Foreground::NONE,
                   ansi::Background::NONE,
                   ansi::Style::FAINT
               );
        oss << "    \"" << symbol.second << "\"\n";
    }
    oss << "\n";
    return oss.str();
}
