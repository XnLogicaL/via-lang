// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "value_ref.h"
#include "interpreter.h"
#include "value.h"

namespace via {

void ValueRef::free() {
  assert(!is_null() && "free called on NULL reference");

  if (--ptr->rc == 0) {
    ptr->free();
    ptr = NULL;
  }
}

bool ValueRef::is_null() const {
  return ptr == NULL;
}

usize ValueRef::count_refs() const {
  assert(!is_null() && "count_refs() called on NULL reference");
  return ptr->rc;
}

ValueRef::ValueRef(Interpreter* ctx) : ptr(NULL) {}
ValueRef::ValueRef(Interpreter* ctx, Value* ptr) : ptr(ptr) {}
ValueRef::~ValueRef() {
  if (!is_null())
    free();
}

ValueRef::ValueRef(const ValueRef& other) : ptr(other.ptr) {
  if (!other.is_null())
    other.ptr->rc++;
}

ValueRef& ValueRef::operator=(const ValueRef& other) {
  if (this != &other) {
    if (!other.is_null())
      other.ptr->rc++;

    if (!is_null())
      free();

    this->ptr = other.ptr;
  }

  return *this;
}

Value* ValueRef::operator->() const {
  assert(!is_null() && "attempt to read NULL reference (member access)");
  return ptr;
}

Value& ValueRef::operator*() const {
  assert(!is_null() && "attempt to read NULL reference (dereference)");
  return *ptr;
}

}  // namespace via
