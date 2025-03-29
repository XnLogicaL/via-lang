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
  vl_assert(reg <= vl_regcount, "invalid register");
  return registers[reg];
}

void state::set_register(operand_t reg, const value_obj& val) {
  vl_assert(reg <= vl_regcount, "invalid register");

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
  string_obj* tstr = new string_obj(this, str);
  push(value_obj(string, tstr));
}

void state::push_table() {
  push(value_obj(table, new table_obj()));
}

void state::push(const value_obj& val) {
  vl_assert(sp < vl_vmstacksize, "stack overflow");
  impl::__push(this, val);
}

void state::drop() {
  vl_assert(sp > 0, "stack underflow");
  impl::__drop(this);
}

value_obj state::pop() {
  vl_assert(sp > 0, "stack underflow");
  return impl::__pop(this);
}

const value_obj& state::top() {
  vl_assert(sp > 0, "stack underflow");
  return sbp[sp];
}

void state::set_stack(size_t position, value_obj value) {
  vl_assert(sp >= position, "stack overflow");
  impl::__set_stack(this, position, value);
}

const value_obj& state::get_stack(size_t position) {
  vl_assert(sp >= position, "stack overflow");
  return impl::__get_stack(this, position);
}

size_t state::stack_size() {
  return sp;
}

value_obj state::to_integer(const value_obj& value) {
  switch (value.type) {
  case value_type::floating_point:
    return value_obj(static_cast<TFloat>(value.val_integer));
  case value_type::boolean:
    return value_obj(value.val_boolean ? 1 : 0);
  case value_type::string: {
    const char* data = value.cast_ptr<string_obj>()->data;

    TInteger stoi_result = std::stoi(data);
    return value_obj(stoi_result);
  }
  default:
    return value_obj();
  }

  vl_unreachable;
}

value_obj state::to_float(const value_obj& value) {
  switch (value.type) {
  case value_type::integer:
    return value_obj(static_cast<TInteger>(value.val_integer));
  case value_type::boolean:
    return value_obj(value.val_boolean ? 1.0f : 0.0f);
  case value_type::string: {
    const char* data = value.cast_ptr<string_obj>()->data;

    TFloat stof_result = std::stof(data);
    return value_obj(stof_result);
  }
  default:
    break;
  }

  return value_obj();
}

value_obj state::to_boolean(const value_obj& value) {
  switch (value.type) {
  case value_type::nil:
    return value_obj(false);
  case value_type::boolean:
    return value_obj(value.val_boolean);
  default:
    break;
  }

  return value_obj(true);
}

value_obj state::to_string(const value_obj&) {
  return value_obj();
}

const value_obj& state::get_global(uint32_t hash) {
  static value_obj nil = value_obj();

  std::lock_guard lock(glb->gtable_mutex);

  auto it = glb->gtable.find(hash);
  if (it != glb->gtable.end()) {
    return it->second;
  }

  return nil;
}

} // namespace via
