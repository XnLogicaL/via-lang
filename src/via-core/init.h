/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <cstdint>
#include <via/config.h>

namespace via {
namespace config {

VIA_CONSTANT size_t PREALLOC_SIZE = 0x8000000ULL; // 128 MiB

} // namespace config

void init(uint8_t verbosity) noexcept;

} // namespace via
