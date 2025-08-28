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

void _assert(bool cond, String message)
{
#ifndef NDEBUG
  (cond ? (void)0 : panic(message));
#endif
}

[[noreturn]] void bug(String what)
{
  debug::_assert(false, fmt::format("internal bug detected: {}", what));
  invoke_ub();
}

[[noreturn]] void todo(String what)
{
  debug::_assert(false, fmt::format("todo: {}", what));
  invoke_ub();
}

[[noreturn]] void unimplemented(String what)
{
  debug::_assert(false, fmt::format("unimplemented: {}", what));
  invoke_ub();
}

}  // namespace debug

}  // namespace via
