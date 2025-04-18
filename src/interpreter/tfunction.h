// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_FUNCTION_H
#define VIA_HAS_HEADER_FUNCTION_H

#include "common.h"
#include "instruction.h"
#include "tvalue.h"

namespace via {

struct State;
struct Closure;
struct CallFrame;

struct UpValue {
  bool is_open = true;
  bool is_valid = false;
  Value* value = nullptr;
  Value heap_value = Value();
};

struct Chunk {
  Instruction* code;
  size_t code_size;
};

struct Function {
  Chunk chunk;
  size_t line_number;
  const char* id;
};

using NativeFn = Value (*)(State* interpreter, Closure* callable);

struct Callable {
  enum class Tag {
    Function,
    Native,
  } type;

  union Un {
    Function fn;
    NativeFn ntv;
  } u;

  size_t arity; // Argc
};

struct Closure {
  Callable callee;
  UpValue* upvs;
  size_t upv_count;

  VIA_IMPLCOPY(Closure);
  VIA_IMPLMOVE(Closure);

  Closure();
  Closure(Function&& fn, size_t arity);
  Closure(NativeFn fn, size_t arity);
  ~Closure();
};

} // namespace via

#endif
