// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "error.h"
#include "vmapi.h"

namespace via {

[[noreturn]] void error_fatal(const char* msg) {
  spdlog::error(msg);
  std::abort();
}

void vmerror(State* S, const char* msg) {
  // check for empty stack:
  if (S->top == S->stk.data)
    error_fatal(msg);

  S->it = INT_ERROR;
  S->err = msg;
}

} // namespace via
