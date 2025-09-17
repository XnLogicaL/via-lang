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
    BytecodeLocal(SymbolId symbol) :
        m_symbol(symbol)
    {}

  public:
    SymbolId get_symbol() const noexcept { return m_symbol; }

  protected:
    SymbolId m_symbol;
};

} // namespace sema
} // namespace via
