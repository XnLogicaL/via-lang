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

// ===========================================================================================
// api.h
//
VIA_NAMESPACE_BEGIN

static const TValue nil;

// ===========================================================================================
// Register manipulation

// Returns a reference to the value that lives in a given register.
TValue& get_register(State&, Operand reg);

// Sets a given register to a given value.
void set_register(State&, Operand reg, const TValue& value);

// ===========================================================================================
// Comparison and metadata

// Returns whether if a given value has a heap-allocated component.
bool is_heap(const TValue& value);

// Compares two given values.
bool compare(const TValue& left, const TValue& right);

// ===========================================================================================
// Basic stack manipulation

// Pushes nil onto the stack.
void push_nil(State& state);

// Pushes an integer onto the stack.
void push_int(State& state, TInteger value);

// Pushes a float onto the stack.
void push_float(State& state, TFloat value);

// Pushes a boolean with value `true` onto the stack.
void push_true(State& state);

// Pushes a boolean with value `false` onto the stack.
void push_false(State& state);

// Pushes a string onto the stack.
void push_string(State& state, const char* str);

// Pushes an empty table onto the stack.
void push_table(State& state);

// Pushes a value onto the stack.
void push(State& state, const TValue& value);

// Drops a value from the stack, frees the resources of the dropped value.
void drop(State& state);

// Pops a value from the stack and returns it.
TValue pop(State& state);

// Returns the top value on the stack.
const TValue& top(State& state);

// ===========================================================================================
// Advanced stack manipulation

// Sets the value at a given position on the stack to a given value.
void set_stack(State& state, size_t position, TValue value);

// Returns the stack value at a given position.
const TValue& get_stack(State& state, size_t position);

// Returns the stack value at a given offset relative to the current stack-frame's stack pointer.
const TValue& get_argument(State& state, size_t offset);

// Returns the size of stack.
size_t stack_size(State& state);

// ===========================================================================================
// Value manipulation

// Attempts to convert a given value into an integer.
TValue to_integer(const TValue& value);

// Attempts to convert a given value into a float.
TValue to_float(const TValue& value);

// Converts a given value into a boolean.
TValue to_boolean(const TValue& value);

// Converts a given value into a string.
TValue to_string(const TValue& value);

// ===========================================================================================
// Table manipulation

// Returns the field that lives in a given hashed key of a given table.
const TValue& get_table(TTable* tbl, u32 key);

// Sets the given tables corresponding hashed key field to a given value.
void set_table(TTable* tbl, u32 key, const TValue& value);

// ===========================================================================================
// Object manipulation

// Returns the field that lives in a given index of a given object.
TValue get_field(TObject* obj, size_t index);

// Returns the method that lives in a given index of a given object.
TValue get_method(TObject* obj, size_t index);

// Returns the constructor of a given object.
TValue get_constructor(TObject* object);

// Returns the destructor of a given object.
TValue get_destructor(TObject* object);

// Returns the given operators overload of a given object.
TValue get_operator_overload(TObject* object, OpCode operation);

// ===========================================================================================
// Global manipulation

// Returns the global that corresponds to a given hashed identifier.
const TValue& get_global(State& state, u32 hash);

// Sets the global that corresponds to a given hashed identifier to a given value.
void set_global(State& state, u32 hash, const TValue& value);

// ===========================================================================================
// Function manipulation

// Standard return. Returns from the current function with an optional value.
void native_return(State& state, const TValue& return_value);

// Calls the given TFunction object with a given argument count.
void native_call(State& state, TFunction* target, size_t argc);

// Calls the method that lives in a given index of a given object with a given argument count.
void method_call(State& state, TObject* object, size_t index, size_t argc);

// Attempts to call the given value object with the given argument count.
void call(State& state, const TValue& callee, size_t argc);

// Returns the current stack frame.
TFunction* get_stack_frame();

// Attempts to return the upvalue that lives in the given index of the given closure.
const TValue& get_upvalue(TFunction* closure, size_t index);

// Attempts to set the upvalue that lives in the given index of the given closure to a given value.
void set_upvalue(TFunction* closure, size_t index, const TValue& value);

// Returns the upvalue count of the given closure.
size_t get_upvalue_count(TFunction* closure);

// Returns the local count of the given closure.
size_t get_local_count_closure(TFunction* closure);

// ===========================================================================================
// General operations

TValue len(State& state, const TValue& target);
TValue arith(State& state, TValue& left, TValue& right, OpCode operation);
TValue concat(State& state, TValue& left, TValue& right);

VIA_NAMESPACE_END

#endif
