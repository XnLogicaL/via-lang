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
#include <spdlog/spdlog.h>
#include <via/config.hpp>

namespace via {
namespace config {

// Logging level for crashes
VIA_CONSTANT const auto CRASH_LOGGER_LEVEL = spdlog::level::err;

#ifdef NDEBUG
VIA_CONSTANT bool DEBUG_ENABLED = false;
#else
VIA_CONSTANT bool DEBUG_ENABLED = true;
#endif

} // namespace config

namespace debug {

// Basically `assert`
void require(bool cond, std::string msg = "<no-message>") noexcept;

[[noreturn]] void panic() noexcept;
[[noreturn]] void bug(std::string msg = "<no-message>") noexcept;
[[noreturn]] void todo(std::string msg = "<no-message>") noexcept;
[[noreturn]] void unimplemented(std::string msg = "<no-message>") noexcept;

} // namespace debug
} // namespace via
