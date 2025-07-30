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
void Stack<T>::push(T val) {
  *(sp++) = val;
}

template <typename T>
T Stack<T>::pop() {
  return *(--sp);
}

template <typename T>
T Stack<T>::top() {
  return sp[-1];  // UB lol
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
