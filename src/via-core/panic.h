/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <spdlog/spdlog.h>
#include <via/config.h>
#include <via/types.h>

namespace via
{

[[noreturn]] void panic(std::string message);

template <typename... Args>
[[noreturn]] void panic(fmt::format_string<Args...> fmt, Args&&... args)
{
  panic(fmt::format(fmt, std::forward<Args>(args)...));
}

}  // namespace via
