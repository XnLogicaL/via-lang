/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <via/config.hpp>
#include "module/symbol.hpp"

namespace via {
namespace sema {

class BytecodeLocal final
{
  public:
    struct Ref
    {
        uint16_t id;
        BytecodeLocal* local;

        Ref() = default;
        Ref(uint16_t id, BytecodeLocal* local) noexcept
            : id(id),
              local(local)
        {}
    };

  public:
    BytecodeLocal() = default;
    BytecodeLocal(SymbolId symbol, size_t version)
        : m_symbol(symbol),
          m_version(version)
    {}

  public:
    SymbolId get_symbol() const noexcept { return m_symbol; }
    size_t get_version() const noexcept { return m_version; }

  protected:
    SymbolId m_symbol;
    size_t m_version;
};

} // namespace sema
} // namespace via
