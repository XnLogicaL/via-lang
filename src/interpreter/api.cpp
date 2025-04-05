// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "common.h"
#include "instruction.h"
#include "opcode.h"
#include "string-utility.h"
#include "api-impl.h"
#include "api-aux.h"
#include "state.h"
#include <cmath>

namespace via {

using enum value_type;

value_obj& state::get_register(operand_t reg) {
  VIA_ASSERT(reg <= VIA_REGCOUNT, "invalid register");
  return registers[reg];
}

void state::set_register(operand_t reg, const value_obj& val) {
  VIA_ASSERT(reg <= VIA_REGCOUNT, "invalid register");

  value_obj* addr = registers + reg;
  *addr = val.clone();
}

bool state::is_heap(const value_obj& value) {
  return static_cast<uint32_t>(value.type) >= static_cast<uint32_t>(value_type::string);
}

bool state::compare(const value_obj& left, const value_obj& right) {
  if (left.type != right.type) {
    return false;
  }

  return true;
}

void state::push_nil() {
  push(value_obj());
}

void state::push_int(TInteger value) {
  push(value_obj(value));
}

void state::push_float(TFloat value) {
  push(value_obj(value));
}

void state::push_true() {
  push(value_obj(true));
}

void state::push_false() {
  push(value_obj(false));
}

void state::push_string(const char* str) {
  push(value_obj(str));
}

void state::push_table() {
  push(value_obj(table, new table_obj()));
}

void state::push(const value_obj& val) {
  VIA_ASSERT(sp < VIA_VMSTACKSIZE, "stack overflow");
  impl::__push(this, val);
}

void state::drop() {
  VIA_ASSERT(sp > 0, "stack underflow");
  impl::__drop(this);
}

value_obj state::pop() {
  VIA_ASSERT(sp > 0, "stack underflow");
  return impl::__pop(this);
}

const value_obj& state::top() {
  VIA_ASSERT(sp > 0, "stack underflow");
  return sbp[sp];
}

void state::set_stack(size_t position, value_obj value) {
  VIA_ASSERT(sp >= position, "stack overflow");
  impl::__set_stack(this, position, value);
}

const value_obj& state::get_stack(size_t position) {
  VIA_ASSERT(sp >= position, "stack overflow");
  return impl::__get_stack(this, position);
}

size_t state::stack_size() {
  return sp;
}

value_obj state::get_global(const char* name) {
  return glb->gtable.get(name);
}

} // namespace via
