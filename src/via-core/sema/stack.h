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
#include <stack>
#include "module/symbol.h"
#include "option.h"

namespace via
{

namespace sema
{

class Module;

template <typename Local>
class Frame final
{
 public:
  using Ref = struct Local::Ref;

 public:
  Local& top() { return mLocals.back(); }

  Option<Ref> getLocal(SymbolId symbol)
  {
    for (i64 i = mLocals.size() - 1; i >= 0; --i) {
      Local& local = mLocals[i];
      if (local.getSymbol() == symbol) {
        return Ref{.id = static_cast<u16>(i), .local = local};
      }
    }

    return nullopt;
  }

  template <typename... Args>
    requires(std::is_constructible_v<Local, SymbolId, Args...>)
  void setLocal(SymbolId symbol, Args&&... args)
  {
    usize version;
    if (auto lref = getLocal(symbol)) {
      version = lref->local.getVersion() + 1;
    } else {
      version = 0;
    }

    mLocals.emplace_back(symbol, args...);
  }

  void save() { mStkPtr = mLocals.size(); }
  void restore() { mLocals.resize(mStkPtr); }

 private:
  Module* mModule;
  usize mStkPtr;
  std::vector<Local> mLocals;
};

template <typename Local>
using StackState = std::stack<Frame<Local>>;

}  // namespace sema

}  // namespace via
