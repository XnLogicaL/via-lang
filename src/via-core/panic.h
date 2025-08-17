// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_PANIC_H_
#define VIA_CORE_PANIC_H_

#include <spdlog/spdlog.h>
#include <via/config.h>
#include <via/types.h>

#define VIA_BUG(msg)                                              \
  panic(                                                          \
      "internal tooling bug detected (please create an issue at " \
      "https://github.com/XnLogicaL/via-lang): " msg)

#define VIA_TODO(msg) panic("TODO: " msg);
#define VIA_UNIMPLEMENTED(msg) panic("unimplemented: " msg);

namespace via {

[[noreturn]] void panic(String message);

template <typename... Args>
[[noreturn]] void panic(fmt::format_string<Args...> fmt, Args&&... args) {
  panic(fmt::format(fmt, std::forward<Args>(args)...));
}

}  // namespace via

#endif
