/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "stack.h"
#include "debug.h"

namespace sema = via::sema;

via::Option<sema::LocalRef> sema::Frame::getLocal(via::SymbolId symbol)
{
  for (i64 i = mLocals.size() - 1; i >= 0; --i) {
    Local& local = mLocals[i];
    if (local.getSymbol() == symbol) {
      return LocalRef{static_cast<u16>(i), local};
    }
  }

  return nullopt;
}

void sema::Frame::setLocal(via::SymbolId symbol,
                           const ast::StmtVarDecl* astDecl,
                           const ir::StmtVarDecl* irDecl,
                           u64 quals)
{
  usize version;
  if (auto lref = getLocal(symbol)) {
    version = lref->local.getVersion() + 1;
  } else {
    version = 0;
  }

  mLocals.emplace_back(symbol, astDecl, irDecl, version, quals);
}
