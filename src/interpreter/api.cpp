// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "state.h"
#include "api-impl.h"
#include <cmath>

namespace via {

using enum Value::Tag;

Value& State::get_register(operand_t reg) {
  return *impl::__get_register(this, reg);
}

void State::set_register(operand_t reg, Value val) {
  impl::__set_register(this, reg, std::move(val));
}

void State::push_nil() {
  push(Value());
}

void State::push_int(int value) {
  push(Value(value));
}

void State::push_float(float value) {
  push(Value(value));
}

void State::push_true() {
  push(Value(true));
}

void State::push_false() {
  push(Value(false));
}

void State::push_string(const char* str) {
  push(Value(str));
}

void State::push_array() {
  push(Value(new struct Array()));
}

void State::push_dict() {
  push(Value(new struct Dict()));
}

void State::push(Value val) {
  CallFrame* current_callframe = impl::__current_callframe(this);
  VIA_ASSERT(current_callframe->locals_size <= CALLFRAME_MAX_LOCALS, "stack overflow");
  impl::__push(this, std::move(val));
}

void State::drop() {
  CallFrame* current_callframe = impl::__current_callframe(this);
  VIA_ASSERT(current_callframe->locals_size > 0, "stack underflow");
  impl::__drop(this);
}

size_t State::stack_size() {
  CallFrame* current_callframe = impl::__current_callframe(this);
  return current_callframe->locals_size;
}

Value& State::get_global(const char* name) {
  return globals->get(name);
}

} // namespace via
