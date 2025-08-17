// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CLI_PANIC_H_
#define VIA_CLI_PANIC_H_

#include <spdlog/spdlog.h>
#include <via/types.h>

namespace via {

namespace cli {

inline void panic(String message) {
  spdlog::error(message);
  throw -1;
}

inline void panic_assert(bool condition, String message) {
  if (!condition) {
    panic(message);
  }
}

}  // namespace cli

}  // namespace via

#endif
