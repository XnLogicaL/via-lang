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

TValue& get_register(State& state, REG_TYPE reg) {
    VIA_ASSERT(reg <= VIA_REGISTER_COUNT, "invalid register");
    return state.registers[reg];
}

void set_register(State& state, REG_TYPE reg, VALUE_TYPE val) {
    VIA_ASSERT(reg <= VIA_REGISTER_COUNT, "invalid register");

    TValue* addr = state.registers + reg;
    *addr        = val.clone();
}

bool is_heap(VALUE_TYPE value) {
    return static_cast<u32>(value.type) >= static_cast<u32>(ValueType::string);
}

bool compare(VALUE_TYPE left, VALUE_TYPE right) {
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

void push_function(State& state) {
    push(state, TValue(ValueType::function, new TFunction()));
}

void push_object(State& state) {
    push(state, TValue(ValueType::object, new TObject()));
}

VIA_NAMESPACE_END
