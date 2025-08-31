// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_INTERPRETER_H_
#define VIA_VM_INTERPRETER_H_

#include <via/config.h>
#include <via/types.h>
#include "debug.h"
#include "header.h"
#include "instruction.h"
#include "stack.h"

namespace via
{

class Value;
class ValueRef;

namespace config
{

namespace vm
{

inline constexpr usize kRegisterCount = UINT16_MAX + 1;

}

}  // namespace config

class Interpreter final
{
 public:
  Interpreter(const Header* H)
      : H(H),
        pc(H->bytecode.data()),
        stack(&alloc),
        regs(config::vm::kRegisterCount)
  {
    debug::assertm(!H->bytecode.empty(), "illformed header");
  }

 public:
  Stack<uptr>& getStack();
  Allocator& getAllocator();

  ValueRef getConstant(u16 id);

  ValueRef pushLocal(ValueRef val);
  ValueRef getLocal(usize mStkPtr);
  void setLocal(usize mStkPtr, ValueRef val);

  void execute();

 private:
  const Header* H;

  Stack<uptr> stack;
  Vec<Value*> regs;
  Vec<Instruction*> lbt;

  Allocator alloc;

  const uptr* fp;  // frame pointer
  const Instruction* pc;
};

}  // namespace via

#endif
