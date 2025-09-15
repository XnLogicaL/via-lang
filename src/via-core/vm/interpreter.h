/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "debug.h"
#include "executable.h"
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

struct Snapshot
{
  const std::unique_ptr<uptr[]> stack;
  const std::unique_ptr<Value*[]> registers;
  const uptr stackPtr;
  const uptr framePtr;
  const Instruction programCounter;
};

class Interpreter final
{
 public:
  // Internal executor
  template <bool, bool>
  friend void __execute(Interpreter*);

 public:
  Interpreter(const Executable* exe)
      : mExecutable(exe),
        pc(exe->bytecode().data()),
        mAlloc(),
        mStack(mAlloc),
        mRegisters(std::make_unique<Value*[]>(config::vm::kRegisterCount))
  {
    debug::require(!exe->bytecode().empty(), "illformed header");
  }

 public:
  Stack<uptr>& getStack() { return mStack; }
  Allocator& getAllocator() { return mAlloc; }

  ValueRef getConstant(u16 id);

  ValueRef pushLocal(ValueRef val);
  ValueRef getLocal(usize mStkPtr);
  void setLocal(usize mStkPtr, ValueRef val);

  void execute();
  void executeOnce();

  Snapshot createStateSnapshot();

 protected:
  const Executable* mExecutable;
  Allocator mAlloc;

  Stack<uptr> mStack;
  std::unique_ptr<Value*[]> mRegisters;

  uptr* sp;        // saved stack pointer
  const uptr* fp;  // frame pointer
  const Instruction* pc;
};

}  // namespace via
