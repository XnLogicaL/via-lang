// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmerr.h"
#include "vmstate.h"
#include "vmapi.h"

namespace via {

namespace vm {

[[noreturn]] void error_fatal(const char* msg) {
  fprintf(stderr, "Fatal error: %s\n", msg);
  abort();
}

void error(const State* S, const char* msg) {
  // check for empty stack:
  if (stack_size(S) == 0)
    error_fatal(msg);

  S->ectx->interrupt = true;
  S->ectx->msg = msg;
}

} // namespace vm

} // namespace via
