// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "stack.h"

namespace via {

namespace core {

namespace vm {

template <typename T>
HeapAllocator* Stack<T>::get_allocator() {
  return alloc;
}

template <typename T>
usize Stack<T>::size() const {
  return sp - bp;
}

template <typename T>
void Stack<T>::jump(T* dst) {
  assert(dst >= bp && dst <= bp + STACK_SIZE - 1 &&
         "stack jump destination out of bounds");
  sp = dst;
}

template <typename T>
void Stack<T>::jump(usize dst) {
  assert(dst < STACK_SIZE && "stack jump destination out of bounds");
  sp = bp + dst;
}

template <typename T>
void Stack<T>::push(T val) {
  *(sp++) = val;
}

template <typename T>
T Stack<T>::pop() {
  return *(--sp);
}

template <typename T>
T* Stack<T>::top() {
  return sp - 1;
}

template <typename T>
T* Stack<T>::at(usize idx) {
  return bp + idx;
}

template <typename T>
T* Stack<T>::begin() {
  return bp;
}

template <typename T>
T* Stack<T>::end() {
  return sp;
}

}  // namespace vm

}  // namespace core

}  // namespace via
