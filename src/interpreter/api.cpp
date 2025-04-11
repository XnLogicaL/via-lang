//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#include "common.h"
#include "instruction.h"
#include "opcode.h"
#include "string-utility.h"
#include "api-impl.h"
#include "api-aux.h"
#include "state.h"
#include <cmath>

namespace via {

using enum IValueType;

IValue& IState::get_register(operand_t reg) {
  return *impl::__get_register(this, reg);
}

void IState::set_register(operand_t reg, IValue val) {
  impl::__set_register(this, reg, std::move(val));
}

void IState::push_nil() {
  push(IValue());
}

void IState::push_int(int value) {
  push(IValue(value));
}

void IState::push_float(float value) {
  push(IValue(value));
}

void IState::push_true() {
  push(IValue(true));
}

void IState::push_false() {
  push(IValue(false));
}

void IState::push_string(const char* str) {
  push(IValue(str));
}

void IState::push_array() {
  push(IValue(new IArray()));
}

void IState::push_dict() {
  push(IValue(new IDict()));
}

void IState::push(IValue val) {
  VIA_ASSERT(sp < VIA_VMSTACKSIZE, "stack overflow");
  impl::__push(this, std::move(val));
}

void IState::drop() {
  VIA_ASSERT(sp > 0, "stack underflow");
  impl::__drop(this);
}

IValue IState::pop() {
  VIA_ASSERT(sp > 0, "stack underflow");
  return impl::__pop(this);
}

const IValue& IState::top() {
  VIA_ASSERT(sp > 0, "stack underflow");
  return sbp[sp];
}

void IState::set_stack(size_t position, IValue value) {
  VIA_ASSERT(sp >= position, "stack overflow");
  impl::__set_stack(this, position, std::move(value));
}

IValue& IState::get_stack(size_t position) {
  VIA_ASSERT(sp >= position, "stack overflow");
  return impl::__get_stack(this, position);
}

size_t IState::stack_size() {
  return sp;
}

IValue& IState::get_global(const char* name) {
  return glb->gtable.get(name);
}

} // namespace via
