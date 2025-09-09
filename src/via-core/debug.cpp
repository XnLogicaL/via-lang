/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "debug.h"
#include <spdlog/sinks/stdout_color_sinks.h>
#include <cpptrace/cpptrace.hpp>
#include <iostream>

#ifdef NDEBUG
  #define __CRASH_IMPL(MSG)
  #define __ASSERT_IMPL(COND, MSG)
  #define __UNREACHABLE() std::unreachable()
#else
  #define __CRASH_IMPL(MSG) (::logError(MSG), ::via::debug::panic())
  #define __ASSERT_IMPL(COND, MSG) (!COND) ? __CRASH_IMPL(MSG) : void(0)
  #define __UNREACHABLE()
#endif

static void logError(std::string message)
{
  static auto stderrLogger = spdlog::stderr_color_mt("std::cerr");
  stderrLogger->log(via::config::kCrashLoggerLevel, message);
}

[[noreturn]] void via::debug::panic() noexcept
{
  logError("program execution raised a panic");
  cpptrace::generate_trace().print(std::cerr);
  std::abort();
}

void via::debug::require(bool cond, std::string message)
{
  __ASSERT_IMPL(
    cond, std::format("program execution reached failing `require()` call: {}",
                      message));
  __UNREACHABLE();
}

[[noreturn]] void via::debug::bug(std::string what)
{
  __ASSERT_IMPL(
    false, std::format("program execution reached `bug()` call: {}", what));
  __UNREACHABLE();
}

[[noreturn]] void via::debug::todo(std::string what)
{
  __ASSERT_IMPL(
    false, std::format("program execution reached `todo()` call: {}", what));
  __UNREACHABLE();
}

[[noreturn]] void via::debug::unimplemented(std::string what)
{
  __ASSERT_IMPL(
    false,
    std::format("program execution reached `unimplemented()` call: {}", what));
  __UNREACHABLE();
}
