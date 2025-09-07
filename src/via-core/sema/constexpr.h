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
#include "const_value.h"
#include "expected.h"
#include "ir/ir.h"

namespace via
{

class Module;

namespace sema
{

bool isConstExpr(Module* m, const ir::Expr* expr);
Expected<ConstValue> evalConstExpr(Module* m, const ir::Expr* expr);

}  // namespace sema

}  // namespace via
