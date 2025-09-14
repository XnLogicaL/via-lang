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
#include "ir/ir.h"

namespace via
{

namespace sema
{

std::vector<const ir::Term*> analyzeControlPaths(
  const ir::StmtBlock* entry) noexcept;
}

}  // namespace via
