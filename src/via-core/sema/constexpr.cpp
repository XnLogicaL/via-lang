/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "constexpr.h"
#include "debug.h"

namespace sema = via::sema;
namespace ir = via::ir;

bool isConstExpr(via::Module* m, const ir::Expr* expr)
{
  via::debug::unimplemented("isConstExpr");
}

via::Expected<sema::ConstValue> evalConstExpr(via::Module* m,
                                              const ir::Expr* expr)
{
  via::debug::unimplemented("evalConstExpr");
}
