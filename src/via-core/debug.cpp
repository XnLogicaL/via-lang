/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "debug.h"
#include "panic.h"

namespace via
{

namespace debug
{

[[noreturn]] static void invoke_ub()
{
  std::unreachable();
}

void assertm(bool cond, std::string message, std::source_location __loc)
{
#ifndef NDEBUG
  if (!cond) {
    panic(message);
  }
#endif
}

[[noreturn]] void bug(std::string what, std::source_location __loc)
{
  debug::assertm(false, fmt::format("internal bug detected: {}", what), __loc);
  invoke_ub();
}

[[noreturn]] void todo(std::string what, std::source_location __loc)
{
  debug::assertm(false, fmt::format("todo: {}", what), __loc);
  invoke_ub();
}

[[noreturn]] void unimplemented(std::string what, std::source_location __loc)
{
  debug::assertm(false, fmt::format("unimplemented: {}", what), __loc);
  invoke_ub();
}

}  // namespace debug

}  // namespace via
