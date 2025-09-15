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
#include "memory.h"

namespace via
{

namespace config
{

namespace vm
{

inline constexpr usize kStackSize = 8192;

}

}  // namespace config

template <typename T>
class Stack final
{
 public:
  Stack(Allocator& pAlloc)
      : mAlloc(pAlloc),
        mBasePtr(mAlloc.alloc<T>(config::vm::kStackSize)),
        mStkPtr(mBasePtr)
  {}

 public:
  Allocator& getAllocator() { return mAlloc; }

  usize size() const { return mStkPtr - mBasePtr; }
  void jump(T* dst) { mStkPtr = dst; }
  void jump(usize dst) { mStkPtr = mBasePtr + dst; }
  void push(T val) { *(mStkPtr++) = val; }
  T pop() { return *(--mStkPtr); }
  T* top() { return mStkPtr - 1; }
  T* at(usize idx) { return mBasePtr + idx; }
  T* begin() { return mBasePtr; }
  T* end() { return mStkPtr - 1; }

 private:
  Allocator& mAlloc;
  T* const mBasePtr;
  T* mStkPtr;
};

}  // namespace via
