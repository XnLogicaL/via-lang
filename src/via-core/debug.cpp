// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "debug.h"
#include "panic.h"

namespace via {

#undef assert

void assert(bool cond, String message) {
#ifndef NDEBUG
  (cond ? (void)0 : panic(message));
#endif
}

[[noreturn]] void bug(String what) {
  assert(false, fmt::format("internal bug detected: {}", what));
  std::unreachable();
}

[[noreturn]] void todo(String what) {
  assert(false, fmt::format("todo: {}", what));
  std::unreachable();
}

[[noreturn]] void unimplemented(String what) {
  assert(false, fmt::format("unimplemented: {}", what));
  std::unreachable();
}

}  // namespace via
