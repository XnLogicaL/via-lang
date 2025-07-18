// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_STATE_H
#define VIA_STATE_H

#include "common.h"
#include "error.h"
#include "heapbuf.h"
#include "mem.h"
#include "vminstr.h"
#include "vmval.h"
#include "vmheader.h"
#include <mimalloc.h>

#define VIA_MAXSTACK  200
#define VIA_MAXCSTACK 200
#define VIA_STATICMEM 1024 * 1024 * 8

namespace via {

enum Interrupt {
  INT_NONE,
  INT_HALT,
  INT_ERROR,
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

  DictValue* gt = NULL;

  HeapBuffer<Value> rf;
  HeapBuffer<Value> stk;
  HeapBuffer<CallInfo> ci_stk;
  HeapBuffer<Instruction*> lt;

  const Instruction* pc = NULL;

  HeapAllocator heap;

  Interrupt it = INT_NONE;
  const char* err = "<error>";

  Value* top;
  CallInfo* ci_top;

  State(const Header& H);
  ~State();

  VIA_NOCOPY(State);
  VIA_NOMOVE(State);
};

} // namespace via

#endif
