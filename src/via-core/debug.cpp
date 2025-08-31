// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

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

void assertm(bool cond, String message, std::source_location __loc)
{
#ifdef NDEBUG
  void(0);
#else
  cond ? panic(message) : void(0);
#endif
}

[[noreturn]] void bug(String what, std::source_location __loc)
{
  debug::assertm(false, fmt::format("internal bug detected: {}", what), __loc);
  invoke_ub();
}

[[noreturn]] void todo(String what, std::source_location __loc)
{
  debug::assertm(false, fmt::format("todo: {}", what), __loc);
  invoke_ub();
}

[[noreturn]] void unimplemented(String what, std::source_location __loc)
{
  debug::assertm(false, fmt::format("unimplemented: {}", what), __loc);
  invoke_ub();
}

}  // namespace debug

}  // namespace via
