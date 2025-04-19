// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_FUNCTION_H
#define VIA_HAS_HEADER_FUNCTION_H

#include "common.h"
#include "instruction.h"
#include "tvalue.h"

namespace via {

inline constexpr size_t CLOSURE_INITIAL_UPV_COUNT = 10;

struct State;
struct Closure;
struct CallFrame;

struct UpValue {
  bool is_open = true;
  bool is_valid = false;
  Value* value = nullptr;
  Value heap_value = Value();
};

struct Function {
  Instruction* code = nullptr;
  size_t code_size = 0;
  size_t line_number = 0;
  const char* id = "<anonymous>";

  VIA_IMPLCOPY(Function);
  VIA_IMPLMOVE(Function);

  Function() = default;
  Function(size_t code_size);
  ~Function();
};

using NativeFn = Value (*)(State* interpreter, Closure* callable);

struct Callable {
  enum class Tag {
    None,
    Function,
    Native,
  } type = Tag::Function;

  union Un {
    Function* fn = nullptr;
    NativeFn ntv;
  } u;

  size_t arity = 0; // Argc

  VIA_IMPLCOPY(Callable);
  VIA_IMPLMOVE(Callable);

  Callable() = default;
  Callable(Function* fn, size_t arity);
  Callable(NativeFn fn, size_t arity);
  ~Callable();
};

struct Closure {
  Callable callee;
  UpValue* upvs = nullptr;
  size_t upv_count = CLOSURE_INITIAL_UPV_COUNT;

  VIA_IMPLCOPY(Closure);
  VIA_IMPLMOVE(Closure);

  Closure();
  ~Closure();
};

} // namespace via

#endif
