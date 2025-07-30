// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_INTERPRETER_H_
#define VIA_VM_INTERPRETER_H_

#include <util/buffer.h>
#include <via/config.h>
#include "header.h"
#include "instruction.h"
#include "stack.h"
#include "value.h"
#include "value_ref.h"

namespace via {

namespace core {

namespace vm {

inline constexpr usize REGISTER_COUNT = UINT16_MAX + 1;

// Lua reference haha
using StkId = Value*;

class Interpreter {
 public:
  constexpr Interpreter(const Header* H)
      : H(H), pc(H->is.data()), stack(&alloc), regs(REGISTER_COUNT) {
    assert(!H->is.empty() && "illformed VM header (bytecode array empty)");
  }

  Stack<uptr>& get_stack();
  HeapAllocator& get_allocator();

  StkId* new_local(Value* val);
  void set_local(StkId* id, Value* val);
  Value* get_local(StkId* id);
  ValueRef get_local_ref(StkId* id);

  void execute_once();
  void execute();

 private:
  const Header* H;

  Stack<uptr> stack;
  Buffer<Value> regs;
  HeapAllocator alloc;

  const uptr* fp;  // frame pointer
  const Instruction* pc;
};

}  // namespace vm

}  // namespace core

}  // namespace via

#endif
