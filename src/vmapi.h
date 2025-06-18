// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_API_IMPL_H
#define VIA_API_IMPL_H

#include "common.h"
#include "vmopc.h"
#include "vmstate.h"
#include "vmdict.h"
#include "vmarr.h"
#include "vmfunc.h"
#include "ustring.h"
#include "memory.h"

namespace via {

namespace vm {

const InstructionData& pcdata(const State* state, const Instruction* const pc);
std::string funcsig(const Callable& func);
std::string nativeid(NativeFn fn);

Value get_constant(const State* state, size_t index);

const char* type(const Value& val);
const void* to_pointer(const Value& val);

void call(State* state, Closure* callee);
void pcall(State* state, Closure* callee);
void ret(State* VIA_RESTRICT state, Value&& retv);

int length(const Value& val);

const char* to_string(const Value& val);

bool to_bool(const Value& val);

int to_int(const State* V, const Value& val);

float to_float(const State* V, const Value& val);

void closure_upvs_resize(Closure* closure);

bool closure_upvs_range_check(Closure* closure, size_t index);

UpValue* closure_upv_get(Closure* closure, size_t upv_id);

void closure_upvs_resize(Closure* closure);

bool closure_upvs_range_check(Closure* closure, size_t index);

UpValue* closure_upv_get(Closure* closure, size_t upv_id);

void closure_upv_set(Closure* closure, size_t upv_id, Value& val);

void closure_init(State* state, Closure* closure, size_t len);

void closure_close_upvalues(const Closure* closure);

size_t dict_hash_key(const Dict* dict, const char* key);

void dict_set(const Dict* dict, const char* key, Value val);

Value* dict_get(const Dict* dict, const char* key);

size_t dict_size(const Dict* dict);

bool array_range_check(const Array* array, size_t index);

void array_resize(Array* array);

void array_set(Array* array, size_t index, Value val);

Value* array_get(const Array* array, size_t index);

size_t array_size(const Array* array);

void label_load(const State* state);

void push(State* state, Value&& val);

void drop(State* state);

Value* get_local(State* VIA_RESTRICT state, size_t offset);

void set_local(State* VIA_RESTRICT state, size_t offset, Value&& val);

void set_register(const State* state, operand_t reg, Value&& val);

Value* get_register(const State* state, operand_t reg);

} // namespace vm

} // namespace via

#endif
