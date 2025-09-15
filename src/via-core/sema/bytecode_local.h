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

namespace via
{

namespace sema
{

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
  BytecodeLocal(SymbolId symbol) : mSymbol(symbol) {}

 public:
  SymbolId getSymbol() const noexcept { return mSymbol; }

 protected:
  SymbolId mSymbol;
};

}  // namespace sema

}  // namespace via
