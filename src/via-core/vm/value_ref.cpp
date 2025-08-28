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
  if (!other.is_null())
    other.ptr->rc++;
}

ValueRef::ValueRef(ValueRef&& other) : ptr(other.ptr)
{
  other.ptr = nullptr;
}

ValueRef::~ValueRef()
{
  if (!is_null())
    free();
}

ValueRef& ValueRef::operator=(const ValueRef& other)
{
  if (this != &other) {
    if (!other.is_null())
      other.ptr->rc++;

    if (!is_null())
      free();

    this->ptr = other.ptr;
  }

  return *this;
}

ValueRef& ValueRef::operator=(ValueRef&& other)
{
  if (this != &other) {
    if (!is_null())
      free();

    ptr = other.ptr;
    other.ptr = nullptr;
  }

  return *this;
}

Value* ValueRef::operator->() const
{
  assert(!is_null() && "attempt to read nullptr reference (member access)");
  return ptr;
}

Value& ValueRef::operator*() const
{
  assert(!is_null() && "attempt to read nullptr reference (dereference)");
  return *ptr;
}

void ValueRef::free()
{
  assert(!is_null() && "free called on nullptr reference");

  if (--ptr->rc == 0) {
    ptr->free();
    ptr = nullptr;
  }
}

bool ValueRef::is_null() const
{
  return ptr == nullptr;
}

usize ValueRef::count_refs() const
{
  assert(!is_null() && "count_refs() called on nullptr reference");
  return ptr->rc;
}

}  // namespace via
