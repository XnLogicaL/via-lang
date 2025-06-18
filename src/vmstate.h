// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_STATE_H
#define VIA_STATE_H

#include <csetjmp>
#include "common.h"
#include "heapbuf.h"
#include "vmcstk.h"
#include "vminstr.h"
#include "vmval.h"
#include "vmerr.h"

namespace via {

namespace vm {

struct alignas(64) State {
  ErrorContext* ectx = NULL;

  Dict* gt = NULL;
  HeapBuffer<Value> stk;
  HeapBuffer<CallInfo> ci_stk;
  HeapBuffer<Instruction*> lt;

  const Instruction* pc = NULL;

  Value* top;
  CallInfo* ci_top;

  VIA_NOCOPY(State);
  VIA_NOMOVE(State);

  State();
  ~State();
};

} // namespace vm

} // namespace via

#endif
