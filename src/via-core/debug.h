/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <functional>
#include <spdlog/spdlog.h>
#include <via/config.h>
#include <via/types.h>

namespace via {
namespace config {

// What to print when debug functions aren't provided a message
VIA_CONSTANT const char* CRASH_LOGGER_NO_MESSAGE = "<no-message>";

// Logging level for crashes
VIA_CONSTANT const auto CRASH_LOGGER_LEVEL = spdlog::level::err;

#ifdef NDEBUG
VIA_CONSTANT bool DEBUG_ENABLED = false;
#else
VIA_CONSTANT bool DEBUG_ENABLED = true;
#endif

} // namespace config

namespace debug {

#define MSG_PARM std::string message = config::CRASH_LOGGER_NO_MESSAGE

// Basically `assert`
void require(bool cond, MSG_PARM) noexcept;
[[noreturn]] void panic() noexcept;
[[noreturn]] void bug(MSG_PARM) noexcept;
[[noreturn]] void todo(MSG_PARM) noexcept;
[[noreturn]] void unimplemented(MSG_PARM) noexcept;

#undef MSG_PARM

template <typename T, char LDel = '{', char RDel = '}'>
inline std::string to_string(
    const std::vector<T>& vec,
    std::function<std::string(const std::remove_cv_t<T>&)> fn
)
{
    std::ostringstream oss;
    oss << LDel;

    for (size_t i = 0; i < vec.size(); i++) {
        oss << fn(vec[i]);
        if (i != vec.size() - 1) {
            oss << ", ";
        }
    }

    oss << RDel;
    return oss.str();
}

} // namespace debug
} // namespace via
