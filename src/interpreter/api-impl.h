// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_VMAPI_H
#define VIA_HAS_HEADER_VMAPI_H

#include "common.h"
#include "constant.h"
#include "tfunction.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "String-utility.h"

namespace via::impl {

// Internal function for throwing errors.
void __set_error_state(const State* state, const std::string& message);

// Internal function for discarding errors.
void __clear_error_state(const State* state);

// Internal function that returns whether if an error has been thrown but not handled.
bool __has_error(const State* state);

/**
 * Internal function that handles errors when thrown. Unwinds the stack until a stack-frame with
 * error-handling capabilities is found. If the main stack-frame is touched, automatically
 * terminates the program.
 */
bool __handle_error(const State* state);

// Internal function used to get the kvalue from the constant table at the given index.
Value __get_constant(const State* state, size_t index);

// Internal function that returns the type of the given object as a String.
Value __type(const Value& val);

// Internal function that returns the type of the given object as an std::string.
std::string __type_cxx(const Value& val);

/**
 * Internal function that returns the underlying pointer of the value if the value type is
 * heap-allocated, nullptr if not.
 */
void* __to_pointer(const Value& val);

CallFrame* __current_callframe(State* state);

/**
 * Internal function that serves as a generalized call interface. Used to call any type.
 * Invokes `__native_call` for native functions, `__extern_call` for C functions.
 */
void __call(State* state, Closure* callee);

// Internal function that returns the length of the given value or Nil if impossible.
Value __length(const Value& val);

// Internal function that returns the length of the given value as a int or -1 if impossible.
int __length_cxx(const Value& val);

/**
 * Internal function that performs a return operation. Restores the stack pointer and program
 * counter to the top-most callframes call_data. After the stack pointer is restored, pushes a
 * return value back onto the stack.
 */
void __return(State* VIA_RESTRICT state, Value&& retv);

// Internal function that reutrns the given value as a String object.
Value __to_string(const Value& val);

// Internal function that returns the given value as an std::string.
std::string __to_cxx_string(const Value& val);

// Internal function that returns the given value as a literal std::string.
std::string __to_literal_cxx_string(const Value& val);

// Internal function that returns the truthy-ness of the given value.
Value __to_bool(const Value& val);

// Internal function that returns the truthy-ness of the given value as a Bool.
bool __to_cxx_bool(const Value& val);

// Internal function that returns the int representation of the given value or Nil if impossible.
Value __to_int(const State* V, const Value& val);

// Internal function that returns the FP representation of the given value or Nil if impossible.
Value __to_float(const State* V, const Value& val);

// Internal function that performs a deep equality comparison between two given values.
bool __compare(const Value& val0, const Value& val1);

// Automatically resizes UpValue vector of closure by VIA_UPV_RESIZE_FACTOR.
void __closure_upvs_resize(Closure* closure);

// Checks if a given index is within the bounds of the UpValue vector of the closure.
// Used for resizing.
bool __closure_upvs_range_check(Closure* closure, size_t index);

// Attempts to retrieve UpValue at index <upv_id>.
// Returns nullptr if <upv_id> is out of UpValue vector bounds.
UpValue* __closure_upv_get(Closure* closure, size_t upv_id);

// Dynamically reassigns UpValue at index <upv_id> the value <val>.
void __closure_upv_set(Closure* closure, size_t upv_id, Value& val);

// Loads closure bytecode by iterating over the Instruction pipeline.
// Handles sentinel/special opcodes like RET or CAPTURE while assembling closure.
void __closure_bytecode_load(State* state, Closure* closure, size_t len);

// Moves upvalues of the current closure into the heap, "closing" them.
void __closure_close_upvalues(const Closure* closure);

// Hashes a dictionary key using the FNV-1a hashing algorithm.
size_t __dict_hash_key(const Dict* dict, const char* key);

// Inserts a key-value pair into the hash table component of a given table_obj object.
void __dict_set(const Dict* dict, const char* key, Value val);

// Performs a look-up on the given table with a given key. Returns nullptr upon lookup failure.
Value* __dict_get(const Dict* dict, const char* key);

// Returns the real size_t of the hashtable component of the given table object.
size_t __dict_size(const Dict* dict);

// Checks if the given index is out of bounds of a given tables array component.
bool __array_range_check(const Array* array, size_t index);

// Dynamically grows and relocates the array component of a given table_obj object.
void __array_resize(Array* array);

// Sets the given index of a table to a given value. Resizes the array component of the table_obj
// object if necessary.
void __array_set(Array* array, size_t index, Value val);

// Attempts to get the value at the given index of the array component of the table. Returns nullptr
// if the index is out of array capacity range.
Value* __array_get(const Array* array, size_t index);

size_t __array_size(const Array* array);

void __label_allocate(State* state, size_t count);

void __label_deallocate(State* state);

Instruction* __label_get(const State* state, size_t index);

void __label_load(const State* state);

void __push(State* state, Value&& val);

void __drop(State* state);

Value* __get_local(State* VIA_RESTRICT state, size_t offset);

void __set_local(State* VIA_RESTRICT state, size_t offset, Value&& val);

void __register_allocate(State* state);

void __register_deallocate(const State* state);

void __set_register(const State* state, operand_t reg, Value val);

Value* __get_register(const State* state, operand_t reg);

Closure* __create_main_function(BytecodeHolder& holder);

} // namespace via::impl

#endif
