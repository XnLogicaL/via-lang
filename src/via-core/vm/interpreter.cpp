// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "interpreter.h"

namespace via {

namespace core {

namespace vm {

Stack<uptr>& Interpreter::get_stack() {
  return stack;
}

HeapAllocator& Interpreter::get_allocator() {
  return alloc;
}

}  // namespace vm

}  // namespace core

}  // namespace via
