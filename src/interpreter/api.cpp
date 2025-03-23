// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "common.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "string-utility.h"
#include "rt-types.h"
#include "api-impl.h"
#include "api-aux.h"
#include "state.h"
#include <cmath>

VIA_NAMESPACE_BEGIN

using enum ValueType;

TValue& State::get_register(Operand reg) {
    VIA_ASSERT(reg <= VIA_REGISTER_COUNT, "invalid register");
    return registers[reg];
}

void State::set_register(Operand reg, const TValue& val) {
    VIA_ASSERT(reg <= VIA_REGISTER_COUNT, "invalid register");

    TValue* addr = registers + reg;
    *addr        = val.clone();
}

bool State::is_heap(const TValue& value) {
    return static_cast<uint32_t>(value.type) >= static_cast<uint32_t>(ValueType::string);
}

bool State::compare(const TValue& left, const TValue& right) {
    if (left.type != right.type) {
        return false;
    }

    return true;
}

void State::push_nil() {
    push(TValue());
}

void State::push_int(TInteger value) {
    push(TValue(value));
}

void State::push_float(TFloat value) {
    push(TValue(value));
}

void State::push_true() {
    push(TValue(true));
}

void State::push_false() {
    push(TValue(false));
}

void State::push_string(const char* str) {
    TString* tstr = new TString(this, str);
    push(TValue(string, tstr));
}

void State::push_table() {
    push(TValue(table, new TTable()));
}

void State::push(const TValue& val) {
    VIA_ASSERT(sp < VIA_VM_STACK_SIZE, "stack overflow");
    impl::__push(this, val);
}

void State::drop() {
    VIA_ASSERT(sp > 0, "stack underflow");
    impl::__drop(this);
}

TValue State::pop() {
    VIA_ASSERT(sp > 0, "stack underflow");
    return impl::__pop(this);
}

const TValue& State::top() {
    VIA_ASSERT(sp > 0, "stack underflow");
    return sbp[sp];
}

void State::set_stack(size_t position, TValue value) {
    VIA_ASSERT(sp >= position, "stack overflow");
    impl::__set_stack(this, position, value);
}

const TValue& State::get_stack(size_t position) {
    VIA_ASSERT(sp >= position, "stack overflow");
    return impl::__get_stack(this, position);
}

size_t State::stack_size() {
    return sp;
}

TValue State::to_integer(const TValue& value) {
    switch (value.type) {
    case ValueType::floating_point:
        return TValue(static_cast<TFloat>(value.val_integer));
    case ValueType::boolean:
        return TValue(value.val_boolean ? 1 : 0);
    case ValueType::string: {
        const char* data = value.cast_ptr<TString>()->data;

        TInteger stoi_result = std::stoi(data);
        return TValue(stoi_result);
    }
    default:
        return TValue();
    }

    VIA_UNREACHABLE;
}

TValue State::to_float(const TValue& value) {
    switch (value.type) {
    case ValueType::integer:
        return TValue(static_cast<TInteger>(value.val_integer));
    case ValueType::boolean:
        return TValue(value.val_boolean ? 1.0f : 0.0f);
    case ValueType::string: {
        const char* data = value.cast_ptr<TString>()->data;

        TFloat stof_result = std::stof(data);
        return TValue(stof_result);
    }
    default:
        break;
    }

    return TValue();
}

TValue State::to_boolean(const TValue& value) {
    switch (value.type) {
    case ValueType::nil:
        return TValue(false);
    case ValueType::boolean:
        return TValue(value.val_boolean);
    default:
        break;
    }

    return TValue(true);
}

TValue State::to_string(const TValue&) {
    return TValue();
}

VIA_NAMESPACE_END
