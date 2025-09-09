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
#include "value.h"

namespace via
{

class Interpreter;
class ValueRef final
{
 public:
  ValueRef(Interpreter* ctx) : mPtr(nullptr) {}
  ValueRef(Interpreter* ctx, Value* mPtr) : mPtr(mPtr) {}

  ValueRef(const ValueRef& other) : mPtr(other.mPtr)
  {
    if (!other.isNullRef()) {
      other.mPtr->mRc++;
    }
  }

  ValueRef(ValueRef&& other) : mPtr(other.mPtr) { other.mPtr = nullptr; }

  ~ValueRef()
  {
    if (!isNullRef()) {
      free();
    }
  }

  ValueRef& operator=(const ValueRef& other)
  {
    if (this != &other) {
      if (!other.isNullRef()) {
        other.mPtr->mRc++;
      }

      if (!isNullRef()) {
        free();
      }

      this->mPtr = other.mPtr;
    }

    return *this;
  }

  ValueRef& operator=(ValueRef&& other)
  {
    if (this != &other) {
      if (!isNullRef()) {
        free();
      }

      mPtr = other.mPtr;
      other.mPtr = nullptr;
    }

    return *this;
  }

  Value* operator->() const
  {
    debug::require(!isNullRef(), "attempt to read NULL reference (operator->)");
    return mPtr;
  }

  Value& operator*() const
  {
    debug::require(!isNullRef(), "attempt to read NULL reference (operator*)");
    return *mPtr;
  }

 public:
  Value* get() const { return mPtr; }
  bool isNullRef() const { return mPtr == nullptr; }

  void free()
  {
    debug::require(!isNullRef(), "free called on NULL reference");

    if (--mPtr->mRc == 0) {
      mPtr->free();
      mPtr = nullptr;
    }
  }

  usize getRefCount() const
  {
    debug::require(!isNullRef(), "getRefCount() called on NULL reference");
    return mPtr->mRc;
  }

 private:
  Value* mPtr;
};

}  // namespace via
