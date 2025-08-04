// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "interpreter.h"
#include "value.h"

namespace via {

namespace core {

namespace vm {

Stack<uptr>& Interpreter::get_stack() {
  return stack;
}

HeapAllocator& Interpreter::get_allocator() {
  return alloc;
}

StkEntry* Interpreter::new_local(Value* val) {
  stack.push((uptr)val);
  return (StkEntry*)stack.top();
}

void Interpreter::set_local(StkEntry* id, Value* val) {
  *id = val;
}

Value* Interpreter::get_local(StkEntry* id) {
  return *id;
}

ValueRef Interpreter::get_local_ref(StkEntry* id) {
  return get_local(id)->make_ref();
}

}  // namespace vm

}  // namespace core

}  // namespace via
