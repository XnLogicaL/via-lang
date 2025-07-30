// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "value_ref.h"

namespace via {

namespace core {

namespace vm {

void ValueRef::free() {
  assert(!is_null() && "free called on NULL reference");

  if (--ptr->rc == 0) {
    // TODO: heap_free(..., (void*)ptr);
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

ValueRef::ValueRef(Interpreter* ctx) : ctx(ctx), ptr(NULL /* TODO */) {}

ValueRef::~ValueRef() {
  if (!is_null())
    free();
}

ValueRef::ValueRef(const ValueRef& other) : ctx(other.ctx), ptr(other.ptr) {
  if (!other.is_null())
    other.ptr->rc++;
}

ValueRef& ValueRef::operator=(const ValueRef& other) {
  if (this != &other) {
    if (!other.is_null())
      other.ptr->rc++;

    if (!is_null())
      free();

    this->ctx = other->ctx;
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

}  // namespace vm

}  // namespace core

}  // namespace via
