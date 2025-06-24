// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_ERR_H
#define VIA_ERR_H

#include <common/common.h>
#include <common/heapbuf.h>
#include "vmstr.h"

namespace via {

struct State;

struct ErrorContext {
  bool interrupt;
  const char* msg;
};

[[noreturn]] void error_fatal(const char* msg);

void error(State* S, const char* msg);

template<typename... Args>
void errorf(State* S, const char* fmt, Args... args) {
  int size = snprintf(NULL, 0, fmt, args...);

  HeapBuffer<char> buf(size + 1);

  snprintf(buf.data, size + 1, fmt, args...);
  error(S, buf.data);
}

} // namespace via

#endif
