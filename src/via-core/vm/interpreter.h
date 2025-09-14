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

class Interpreter final
{
 public:
  Interpreter(const Executable* exe)
      : mExecutable(exe),
        pc(exe->bytecode().data()),
        mStack(&mAlloc),
        mRegisters(config::vm::kRegisterCount)
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

 private:
  const Executable* mExecutable;

  Stack<uptr> mStack;
  std::vector<Value*> mRegisters;

  Allocator mAlloc;

  uptr* sp;        // saved stack pointer
  const uptr* fp;  // frame pointer
  const Instruction* pc;
};

}  // namespace via
