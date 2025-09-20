/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "module/symbol.h"

namespace via {
namespace sema {

class BytecodeLocal final
{
  public:
    struct Ref
    {
        u16 id;
        BytecodeLocal& local;
    };

  public:
    BytecodeLocal() = default;
    BytecodeLocal(SymbolId symbol, usize version)
        : m_symbol(symbol),
          m_version(version)
    {}

  public:
    SymbolId get_symbol() const noexcept { return m_symbol; }
    usize get_version() const noexcept { return m_version; }

  protected:
    SymbolId m_symbol;
    usize m_version;
};

} // namespace sema
} // namespace via
