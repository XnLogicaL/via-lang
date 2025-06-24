// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_API_IMPL_H
#define VIA_API_IMPL_H

#include "common.h"
#include "vminstr.h"
#include "vmstate.h"
#include "vmfunc.h"
#include "ustring.h"
#include "memory.h"

namespace via {

namespace vm {

inline int stack_size(const State* S) {
  return S->ci_top - S->ci_stk.data;
}

const InstructionData& pcdata(State* S, const Instruction* pc);

Value get_constant(State* S, size_t index);

void set_register(State* S, uint16_t reg, Value&& val);

Value* get_register(State* S, uint16_t reg);

void push(State* S, Value&& val);

void pop(State* S);

Value* get_local(State* S, size_t offset);

void set_local(State* S, size_t offset, Value&& val);

Value* get_argument(State* S, size_t offset);

Value* get_global(State* S, const char* name);

void set_global(State* S, const char* name, Value&& val);

const char* type(State* S, Value* val);

const void* to_pointer(State* S, Value* val);

void call(State* S, Closure* callee);

void pcall(State* S, Closure* callee);

void ret(State* S, Value&& retv);

int length(State* S, Value* val);

const char* to_string(State* S, Value* val);

bool to_bool(State* S, Value* val);

int to_int(State* S, Value* val);

float to_float(State* S, Value* val);

void label_load(State* S);

} // namespace vm

} // namespace via

#endif
