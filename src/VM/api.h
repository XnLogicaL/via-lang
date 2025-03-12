// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_API_H
#define _VIA_API_H

#include "common.h"
#include "gc.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "rttypes.h"
#include <cmath>

#define VALUE_TYPE   const TValue&
#define CLOSURE_TYPE TFunction*
#define TABLE_TYPE   TTable*
#define OBJECT_TYPE  TObject*

#define HASH_TYPE u32
#define REG_TYPE  Operand
#define STK_ID    size_t

// ===========================================================================================
// api.h
//
VIA_NAMESPACE_BEGIN

static const TValue nil = TValue();

// ===========================================================================================
// Register manipulation

TValue& get_register(State&, REG_TYPE reg);
void    set_register(State&, REG_TYPE reg, VALUE_TYPE value);

// ===========================================================================================
// Comparison and metadata

bool is_heap(VALUE_TYPE value);
bool compare(VALUE_TYPE left, VALUE_TYPE right);

// ===========================================================================================
// Basic stack manipulation

void push_nil(State& state);
void push_int(State& state, TInteger value);
void push_float(State& state, TFloat value);
void push_true(State& state);
void push_false(State& state);
void push_string(State& state, const char* str);
void push_table(State& state);
void push_function(State& state);
void push_object(State& state);

void   replace(State& state, STK_ID position, VALUE_TYPE replacement);
void   push(State& state, VALUE_TYPE value);
void   drop(State& state);
TValue pop(State& state);
TValue top(State& state);

// ===========================================================================================
// Advanced stack manipulation

void       set_stack(State& state, VALUE_TYPE value);
VALUE_TYPE get_stack(State& state);
VALUE_TYPE get_argument(State& state, STK_ID offset);

size_t get_local_count();

// ===========================================================================================
// Value manipulation

TValue to_integer(VALUE_TYPE value);
TValue to_float(VALUE_TYPE value);
TValue to_boolean(VALUE_TYPE value);
TValue to_string(VALUE_TYPE value);

// ===========================================================================================
// Table manipulation

TValue get_table(TABLE_TYPE tbl, HASH_TYPE key);
void   set_table(TABLE_TYPE tbl, HASH_TYPE key, VALUE_TYPE value);

// ===========================================================================================
// Object manipulation

TValue get_field(OBJECT_TYPE obj, size_t index);
TValue get_method(OBJECT_TYPE obj, size_t index);
TValue get_constructor(OBJECT_TYPE object);
TValue get_destructor(OBJECT_TYPE object);
TValue get_operator_overload(OBJECT_TYPE object, OpCode operation);

// ===========================================================================================
// Global manipulation

VALUE_TYPE get_global(State& state, HASH_TYPE hash);
void       set_global(State& state, HASH_TYPE hash, VALUE_TYPE value);

// ===========================================================================================
// Function manipulation

void native_return(State& state, VALUE_TYPE return_value);
void native_call(State& state, CLOSURE_TYPE target, size_t argc);
void method_call(State& state, TABLE_TYPE table, HASH_TYPE key, size_t argc);
void call(State& state, VALUE_TYPE callee, size_t argc);

CLOSURE_TYPE get_stack_frame();
VALUE_TYPE   get_upvalue(CLOSURE_TYPE closure);
void         set_upvalue(CLOSURE_TYPE closure, STK_ID index, VALUE_TYPE value);
size_t       get_upvalue_count(CLOSURE_TYPE closure);
size_t       get_local_count_closure();

// ===========================================================================================
// General operations

TValue len(State& state, VALUE_TYPE target);
TValue arith(State& state, TValue& left, TValue& right, OpCode operation);
TValue concat(State& state, TValue& left, TValue& right);

VIA_NAMESPACE_END

#endif
