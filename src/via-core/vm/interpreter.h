// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_INTERPRETER_H_
#define VIA_VM_INTERPRETER_H_

#include <via/config.h>
#include "buffer.h"
#include "header.h"
#include "instruction.h"
#include "stack.h"


namespace via {

namespace core {

namespace vm {

struct Value;
struct ValueRef;

namespace cfg {

inline constexpr usize REGISTER_COUNT = UINT16_MAX + 1;

}

class Interpreter {
 public:
  constexpr Interpreter(const Header* H)
      : H(H), pc(H->is.data()), stack(&alloc), regs(cfg::REGISTER_COUNT) {
    assert(!H->is.empty() && "illformed header (bytecode array empty)");
  }

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

}  // namespace vm

}  // namespace core

}  // namespace via

#endif
