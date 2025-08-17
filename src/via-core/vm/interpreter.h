// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_INTERPRETER_H_
#define VIA_VM_INTERPRETER_H_

#include <via/config.h>
#include <via/types.h>
#include "buffer.h"
#include "constexpr_ipow.h"
#include "header.h"
#include "instruction.h"
#include "stack.h"

namespace via {

class Value;
class ValueRef;

namespace config {

namespace vm {

inline constexpr usize register_count = UINT16_MAX + 1;

}

}  // namespace config

class Interpreter final {
 public:
  Interpreter(const Header* H)
      : H(H),
        pc(H->bytecode.data()),
        stack(&alloc),
        regs(config::vm::register_count) {
    assert(!H->bytecode.empty() && "illformed header (bytecode array empty)");
  }

 public:
  Stack<uptr>& get_stack();
  HeapAllocator& get_allocator();

  ValueRef get_constant(u16 id);

  ValueRef get_register(u16 reg);
  void set_register(u16 reg, ValueRef val);

  ValueRef new_local(ValueRef val);
  ValueRef get_local(usize sp);
  void set_local(usize sp, ValueRef val);

  void execute();

 private:
  const Header* H;

  Stack<uptr> stack;
  Buffer<Value*> regs;
  Buffer<Instruction*> lbt;

  HeapAllocator alloc;

  const uptr* fp;  // frame pointer
  const Instruction* pc;
};

}  // namespace via

#endif
