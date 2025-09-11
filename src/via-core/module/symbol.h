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
#include <deque>
#include <sstream>
#include "intern_table.h"

namespace via
{

using SymbolId = u64;
using QualName = std::deque<std::string>;

inline std::string toString(const QualName& path)
{
  std::ostringstream oss;

  for (usize i = 0; i < path.size(); i++) {
    if (i > 0) {
      oss << "::";
    }

    oss << path[i];
  }

  return oss.str();
}

class SymbolTable final : public InternTable<std::string, SymbolId>
{
 public:
  using InternTable::intern;

  const auto& getSymbols() const { return mMap; }
  SymbolId intern(const QualName& path) { return intern(toString(path)); }
};

}  // namespace via
