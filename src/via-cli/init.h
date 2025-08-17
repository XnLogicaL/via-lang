// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CLI_INIT_H_
#define VIA_CLI_INIT_H_

#include <spdlog/spdlog.h>

namespace via {

namespace cli {

inline void init() {
  spdlog::set_pattern("%^%l:%$ %v");
}

}  // namespace cli

}  // namespace via

#endif
