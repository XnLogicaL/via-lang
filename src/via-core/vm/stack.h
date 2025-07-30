// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_STACK_H_
#define VIA_VM_STACK_H_

#include <util/memory.h>
#include <via/config.h>

namespace via {

namespace core {

namespace vm {

inline constexpr usize STACK_SIZE = 8192;

template <typename T>
class Stack {
 public:
  constexpr Stack(HeapAllocator* alloc)
      : alloc(alloc), bp(alloc->alloc<T>(STACK_SIZE)), sp(bp) {}

  HeapAllocator* get_allocator();

  usize size() const;

  void jump(T* dst);
  void jump(usize dst);

  void push(T val);
  T pop();
  T* top();
  T* at(usize idx);
  T* begin();
  T* end();

 private:
  HeapAllocator* alloc;
  T* const bp;
  T* sp;
};

}  // namespace vm

}  // namespace core

}  // namespace via

#endif
