// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_STATE_H
#define VIA_STATE_H

#include <csetjmp>
#include <arena.h>
#include "common.h"
#include "heapbuf.h"
#include "vminstr.h"
#include "vmval.h"
#include "vmerr.h"
#include "vmheader.h"

namespace via {

namespace vm {

struct CallInfo {
  int nargs = 0;
  bool protect = false;
  Value* base = NULL;
  Closure* closure = NULL;
  const Instruction* savedpc = NULL;
};

struct alignas(64) State {
  const Header H;

  Dict* gt = NULL;

  HeapBuffer<Value> rf;
  HeapBuffer<Value> stk;
  HeapBuffer<CallInfo> ci_stk;
  HeapBuffer<Instruction*> lt;

  const Instruction* pc = NULL;

  ArenaAllocator ator;
  ErrorContext* ectx = NULL;

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
