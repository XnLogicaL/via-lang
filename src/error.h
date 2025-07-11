// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_ERR_H
#define VIA_ERR_H

#include "common.h"
#include "heapbuf.h"
#include <spdlog/spdlog.h>

namespace via {

struct State;

[[noreturn]] void error_fatal(const char* msg);

void vmerror(State* S, const char* msg);

template<typename... Args>
void vmerrorf(State* S, Fmt<Args...> fmt, Args... args) {
  String buf = std::vformat(fmt, std::forward<Args>(args)...);
  vmerror(S, buf.c_str());
}

} // namespace via

#endif
