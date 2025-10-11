/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <vector>
#include <via/config.hpp>
#include "ir/ir.hpp"

namespace via {
namespace sema {

std::vector<const ir::Term*> get_control_paths(const ir::StmtBlock* entry) noexcept;

}
} // namespace via
