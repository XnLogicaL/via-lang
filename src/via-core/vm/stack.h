// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_STACK_H_
#define VIA_VM_STACK_H_

#include <via/config.h>
#include <via/types.h>
#include "memory.h"

namespace via {

namespace config {

namespace vm {

inline constexpr usize stack_size = 8192;

}

}  // namespace config

template <typename T>
class Stack final {
 public:
  Stack(Allocator* alloc)
      : alloc(alloc), bp(alloc->alloc<T>(config::vm::stack_size)), sp(bp) {}

 public:
  Allocator* get_allocator() { return alloc; }

  usize size() const { return sp - bp; }
  void jump(T* dst) { sp = dst; }
  void jump(usize dst) { sp = bp + dst; }
  void push(T val) { *(sp++) = val; }
  T pop() { return *(--sp); }
  T* top() { return sp - 1; }
  T* at(usize idx) { return bp + idx; }
  T* begin() { return bp; }
  T* end() { return sp - 1; }

 private:
  Allocator* alloc;
  T* const bp;
  T* sp;
};

}  // namespace via

#endif
