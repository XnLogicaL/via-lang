// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmfunc.h"

namespace via {

Closure closure_new(State*, NativeFn fun, usize upvc) {
  Closure C;
  C.buf = UpvBuf(upvc);
  C.native = true;
  C.u = {.nat = fun};

  return C;
}

Closure closure_new(State*, Function* fun, usize upvc) {
  Closure C;
  C.buf = UpvBuf(upvc);
  C.native = true;
  C.u = {.fun = fun};

  return C;
}

void closure_close(State*, Closure* C) {
  if (!C->native)
    delete C->u.fun;
}

bool closure_cmp(State*, Closure* left, Closure* right) {
  return left == right;
}

} // namespace via
