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
#include "option.h"

namespace via
{

namespace sema
{

class Frame final
{
 public:
  Local& top() { return mLocals.back(); }

  Option<LocalRef> getLocal(std::string_view symbol);
  void setLocal(std::string_view symbol,
                const ast::Expr* lval,
                const ast::Expr* rval,
                const sema::Type* type,
                u64 quals = 0ULL);

  void save() { mStkPtr = mLocals.size(); }
  void restore() { mLocals.resize(mStkPtr); }

 private:
  usize mStkPtr;
  Vec<Local> mLocals;
};

using StackState = std::stack<Frame>;

}  // namespace sema

}  // namespace via
