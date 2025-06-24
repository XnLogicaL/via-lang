// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_FUNCTION_H
#define VIA_FUNCTION_H

#include "common.h"
#include "heapbuf.h"
#include "vminstr.h"
#include "vmval.h"

namespace via {

namespace vm {

using NativeFn = Value (*)(State*);

struct State;
struct Value;
struct Closure;
struct CallInfo;

struct UpValue {
  bool open = true;
  bool valid = false;
  Value* value = NULL;
  Value _value;
  State* S = NULL;

  // for HeapBuffer compatability
  inline ~UpValue() {
    value_close(S, &_value);
  }
};

using UpvBuf = HeapBuffer<UpValue>;

struct Function {
  const Instruction* code = NULL;
  size_t code_size = 0;
  size_t line = 0;
  const char* id = "<anonymous>";
};

union ClosureUn {
  Function* fun;
  NativeFn nat;
};

struct Closure {
  UpvBuf buf;
  ClosureUn u;
  bool native = false;
};

Closure closure_new(State* S, NativeFn fun, size_t upvc);
Closure closure_new(State* S, Function* fun, size_t upvc);
void closure_close(State* S, Closure* C);
bool closure_cmp(State* S, Closure* left, Closure* right);

} // namespace vm

} // namespace via

#endif
