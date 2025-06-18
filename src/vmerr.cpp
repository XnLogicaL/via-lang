// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmerr.h"
#include "vmstate.h"

namespace via {

namespace vm {

void error_fatal(const char* msg) {
  std::fprintf(stderr, "Fatal error: %s\n", msg);
  std::abort();
}

void error(const State* S, const char* msg) {
  if (!S->ectx) {
    error_fatal(msg);
  }

  S->ectx->msg = msg;
  std::longjmp(S->ectx->env, 1);
}

void error_toobig(State* S) {
  error(S, "memory allocation error: block too big");
}

void error_outofbounds(State* S) {
  error(S, "mutation error: out of bounds");
}

} // namespace vm

} // namespace via
