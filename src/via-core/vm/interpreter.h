// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_INTERPRETER_H_
#define VIA_VM_INTERPRETER_H_

#include <via/config.h>
#include "instruction.h"
#include "stack.h"
#include "value.h"
#include "value_ref.h"

namespace via {

namespace core {

namespace vm {

class Interpreter {
 public:
  Interpreter() : stack(&alloc) {}

  Stack<uptr>& get_stack();
  HeapAllocator& get_allocator();

 private:
  Stack<uptr> stack;
  HeapAllocator alloc;

  const Instruction* pc;
};

}  // namespace vm

}  // namespace core

}  // namespace via

#endif
