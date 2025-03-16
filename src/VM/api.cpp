// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "common.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "strutils.h"
#include "rttypes.h"
#include "vmapi.h"
#include "api.h"
#include <cmath>

VIA_NAMESPACE_BEGIN

TValue& get_register(State& state, Operand reg) {
    VIA_ASSERT(reg <= VIA_REGISTER_COUNT, "invalid register");
    return state.registers[reg];
}

void set_register(State& state, Operand reg, const TValue& val) {
    VIA_ASSERT(reg <= VIA_REGISTER_COUNT, "invalid register");

    TValue* addr = state.registers + reg;
    *addr        = val.clone();
}

bool is_heap(const TValue& value) {
    return static_cast<u32>(value.type) >= static_cast<u32>(ValueType::string);
}

bool compare(const TValue& left, const TValue& right) {
    if (left.type != right.type) {
        return false;
    }

    return true;
}

void push_nil(State& state) {
    push(state, TValue());
}

void push_int(State& state, TInteger value) {
    push(state, TValue(value));
}

void push_float(State& state, TFloat value) {
    push(state, TValue(value));
}

void push_true(State& state) {
    push(state, TValue(true));
}

void push_false(State& state) {
    push(state, TValue(false));
}

void push_string(State& state, const char* str) {
    push(state, TValue(new TString(&state, str)));
}

void push_table(State& state) {
    push(state, TValue(ValueType::table, new TTable()));
}

void push(State& state, const TValue& val) {
    VIA_ASSERT(state.sp < VIA_VM_STACK_SIZE, "stack overflow");
    impl::__push(&state, val);
}

void drop(State& state) {
    VIA_ASSERT(state.sp > 0, "stack underflow");
    impl::__drop(&state);
}

TValue pop(State& state) {
    VIA_ASSERT(state.sp > 0, "stack underflow");
    return impl::__pop(&state);
}

const TValue& top(State& state) {
    VIA_ASSERT(state.sp > 0, "stack underflow");
    return state.sbp[state.sp];
}

void set_stack(State& state, size_t position, TValue value) {
    VIA_ASSERT(state.sp >= position, "stack overflow");
    impl::__set_stack(&state, position, value);
}

const TValue& get_stack(State& state, size_t position) {
    VIA_ASSERT(state.sp >= position, "stack overflow");
    return impl::__get_stack(&state, position);
}

size_t stack_size(State& state) {
    return state.sp;
}

TValue to_integer(const TValue& value) {
    switch (value.type) {
    case ValueType::floating_point:
        return TValue(static_cast<TFloat>(value.val_integer));
    case ValueType::boolean:
        return TValue(value.val_boolean ? 1 : 0);
    case ValueType::string:
        try {
            const char* data        = value.cast_ptr<TString>()->data;
            TInteger    stoi_result = std::stoi(data);
            return TValue(stoi_result);
        }
        catch (const std::exception& e) {
            VIA_ASSERT(false, std::format("failed to convert to integer: {}", e.what()));
        }

        VIA_UNREACHABLE;
    default:
        return TValue();
    }

    VIA_UNREACHABLE;
}

TValue to_float(const TValue& value) {
    switch (value.type) {
    case ValueType::integer:
        return TValue(static_cast<TInteger>(value.val_integer));
    case ValueType::boolean:
        return TValue(value.val_boolean ? 1.0f : 0.0f);
    case ValueType::string:
        try {
            const char* data        = value.cast_ptr<TString>()->data;
            TFloat      stof_result = std::stof(data);
            return TValue(stof_result);
        }
        catch (const std::exception& e) {
            VIA_ASSERT(false, std::format("failed to convert to float: {}", e.what()));
        }
    default:
        break;
    }

    return TValue();
}

TValue to_boolean(const TValue& value) {
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

TValue to_string(const TValue& value) {}

VIA_NAMESPACE_END
