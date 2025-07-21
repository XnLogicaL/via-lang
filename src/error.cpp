// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "error.h"

namespace via {

[[noreturn]] void error_fatal(const char* msg) {
  spdlog::error(msg);
  std::abort();
}

} // namespace via
