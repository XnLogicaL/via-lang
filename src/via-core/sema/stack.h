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
#include "local.h"
#include "module/symbol.h"
#include "option.h"

namespace via
{

namespace sema
{

class Module;

class Frame final
{
 public:
  Local& top() { return mLocals.back(); }

  Option<LocalRef> getLocal(SymbolId symbol);
  void setLocal(SymbolId symbol,
                const ast::Stmt* astDecl,
                const ir::Stmt* irDecl,
                u8 quals = 0ULL);

  void save() { mStkPtr = mLocals.size(); }
  void restore() { mLocals.resize(mStkPtr); }

 private:
  Module* mModule;
  usize mStkPtr;
  std::vector<Local> mLocals;
};

using StackState = std::stack<Frame>;

}  // namespace sema

}  // namespace via
