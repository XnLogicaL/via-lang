/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "debug.hpp"
#include <cpptrace/cpptrace.hpp>
#include <iostream>
#include <spdlog/sinks/stdout_color_sinks.h>

#ifdef NDEBUG
    #define __CRASH_IMPL(MSG)
    #define __ASSERT_IMPL(COND, MSG)
    #define __UNREACHABLE() std::unreachable()
#else
    #define __CRASH_IMPL(MSG) (::log_error(MSG), ::via::debug::panic())
    #define __ASSERT_IMPL(COND, MSG) (!COND) ? __CRASH_IMPL(MSG) : void(0)
    #define __UNREACHABLE()
#endif

static void log_error(std::string message)
{
    static auto stderrLogger = spdlog::stderr_color_mt("std::cerr");
    stderrLogger->log(via::config::CRASH_LOGGER_LEVEL, message);
}

[[noreturn]] void via::debug::panic() noexcept
{
    log_error("program execution panicked");
    cpptrace::generate_trace().print(std::cerr);
    std::abort();
}

void via::debug::require(bool cond, std::string message) noexcept
{
    __ASSERT_IMPL(
        cond,
        std::format(
            "program execution reached failing `debug::require()` call: {}",
            message
        )
    );
    __UNREACHABLE();
}

[[noreturn]] void via::debug::bug(std::string message) noexcept
{
    __ASSERT_IMPL(
        false,
        std::format("program execution reached `debug::bug()` call: {}", message)
    );
    __UNREACHABLE();
}

[[noreturn]] void via::debug::todo(std::string message) noexcept
{
    __ASSERT_IMPL(
        false,
        std::format("program execution reached `debug::todo()` call: {}", message)
    );
    __UNREACHABLE();
}

[[noreturn]] void via::debug::unimplemented(std::string message) noexcept
{
    __ASSERT_IMPL(
        false,
        std::format(
            "program execution reached `debug::unimplemented()` call: {}",
            message
        )
    );
    __UNREACHABLE();
}
