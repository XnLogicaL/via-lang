/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "panic.h"

namespace via
{

[[noreturn]] void panic(std::string message)
{
  spdlog::error("panic: {}", message);
  std::terminate();
}

}  // namespace via
