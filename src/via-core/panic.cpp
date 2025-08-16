// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "panic.h"

namespace via {

[[noreturn]] void panic(String message) {
  spdlog::error("program panic: {}", message);
  std::terminate();
}

}  // namespace via
