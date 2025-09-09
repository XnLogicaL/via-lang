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
#include <functional>

namespace via
{

namespace config
{

// What to print when debug functions aren't provided a message
inline constexpr const char* kCrashLoggerNoMessage = "<no-message>";

// Logging level for crashes
inline constexpr spdlog::level::level_enum kCrashLoggerLevel =
  spdlog::level::err;

#ifdef NDEBUG
inline constexpr bool kDebugEnabled = false;
#else
inline constexpr bool kDebugEnabled = true;
#endif

}  // namespace config

namespace debug
{

// Basically `assert`
void require(bool cond, std::string message = config::kCrashLoggerNoMessage);

[[noreturn]] void panic() noexcept;
[[noreturn]] void bug(std::string what);
[[noreturn]] void todo(std::string what = config::kCrashLoggerNoMessage);
[[noreturn]] void unimplemented(
  std::string what = config::kCrashLoggerNoMessage);

template <typename T, char LDel = '{', char RDel = '}'>
std::string dump(const Vec<T>& vec,
                 std::function<std::string(const std::remove_cv_t<T>&)> fn)
{
  std::ostringstream oss;
  oss << LDel;

  for (usize i = 0; i < vec.size(); i++) {
    oss << fn(vec[i]);
    if (i != vec.size() - 1) {
      oss << ", ";
    }
  }

  oss << RDel;
  return oss.str();
}

}  // namespace debug

}  // namespace via
