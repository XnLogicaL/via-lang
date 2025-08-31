// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "value_ref.h"
#include "interpreter.h"
#include "value.h"

namespace via
{

ValueRef::ValueRef(Interpreter* ctx) : ptr(nullptr) {}
ValueRef::ValueRef(Interpreter* ctx, Value* ptr) : ptr(ptr) {}

ValueRef::ValueRef(const ValueRef& other) : ptr(other.ptr)
{
  if (!other.isNullRef()) {
    other.ptr->mRc++;
  }
}

ValueRef::ValueRef(ValueRef&& other) : ptr(other.ptr)
{
  other.ptr = nullptr;
}

ValueRef::~ValueRef()
{
  if (!isNullRef()) {
    free();
  }
}

ValueRef& ValueRef::operator=(const ValueRef& other)
{
  if (this != &other) {
    if (!other.isNullRef()) {
      other.ptr->mRc++;
    }

    if (!isNullRef()) {
      free();
    }

    this->ptr = other.ptr;
  }

  return *this;
}

ValueRef& ValueRef::operator=(ValueRef&& other)
{
  if (this != &other) {
    if (!isNullRef()) {
      free();
    }

    ptr = other.ptr;
    other.ptr = nullptr;
  }

  return *this;
}

Value* ValueRef::operator->() const
{
  debug::assertm(!isNullRef(), "attempt to read NULL reference (operator->)");
  return ptr;
}

Value& ValueRef::operator*() const
{
  debug::assertm(!isNullRef(), "attempt to read NULL reference (operator*)");
  return *ptr;
}

void ValueRef::free()
{
  debug::assertm(!isNullRef(), "free called on NULL reference");

  if (--ptr->mRc == 0) {
    ptr->free();
    ptr = nullptr;
  }
}

bool ValueRef::isNullRef() const
{
  return ptr == nullptr;
}

usize ValueRef::getRefCount() const
{
  debug::assertm(!isNullRef(), "getRefCount() called on NULL reference");
  return ptr->mRc;
}

}  // namespace via
