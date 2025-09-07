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

namespace via
{

namespace config
{

inline constexpr usize kRegisterCount = UINT16_MAX;

}

namespace sema
{

namespace registers
{

void reset();
u16 alloc();
void free(u16 reg);

}  // namespace registers

}  // namespace sema

}  // namespace via
