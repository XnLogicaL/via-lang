// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_STATE_H
#define VIA_STATE_H

#include "common.h"
#include "heapbuf.h"
#include "vminstr.h"
#include "vmval.h"
#include "vmerr.h"
#include "vmheader.h"
#include <arena/arena.h>
#include <mimalloc.h>

#define VIA_MAXSTACK  200
#define VIA_MAXCSTACK 200
#define VIA_STATICMEM 1024 * 1024 * 8

namespace via {

enum Interrupt {
  INT_NONE,
  INT_HALT,
};

struct CallInfo {
  int nargs = 0;
  bool protect = false;
  Value* base = NULL;
  Closure* closure = NULL;
  const Instruction* savedpc = NULL;
};

struct alignas(64) State {
  const Header& H;

  Dict* gt = NULL;

  HeapBuffer<Value> rf;
  HeapBuffer<Value> stk;
  HeapBuffer<CallInfo> ci_stk;
  HeapBuffer<Instruction*> lt;

  const Instruction* pc = NULL;

  mi_heap_t* heap;
  ArenaAllocator ator;

  Interrupt it = INT_NONE;
  ErrorContext* e = NULL;

  Value* top;
  CallInfo* ci_top;

  VIA_NOCOPY(State);
  VIA_NOMOVE(State);

  State(const Header& H);
  ~State();
};

} // namespace via

#endif
