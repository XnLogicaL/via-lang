// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file api-impl.h
 * @brief Internal interpreter API implementation
 * @details Contains functions used by the via interpreter engine, including stack
 *          operations, value conversion, function calling, error propagation, closure management,
 *          dictionary/array manipulation, and register management.
 */
#ifndef VIA_HAS_HEADER_API_IMPL_H
#define VIA_HAS_HEADER_API_IMPL_H

#include "common.h"
#include "opcode.h"
#include "state.h"
#include "tdict.h"
#include "tarray.h"
#include "tfunction.h"

#include <ustring.h>
#include <memory.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @namespace impl
 * @defgroup impl_namespace
 * @{
 */
namespace impl {

constexpr inline register_t BACKEND_REGS_START = REGISTER_COUNT - 1024;
constexpr inline register_t BACKEND_REGS_END = REGISTER_COUNT - 1;

const InstructionData& __pcdata(const State* state, const Instruction* const pc);
std::string __funcsig(const Callable& func);
std::string __nativeid(NativeFn fn);

/**
 * @brief Sets the interpreter into an error state with a given message.
 * @param state Interpreter state.
 * @param message The error message.
 */
void __set_error_state(const State* state, const std::string& message);

/**
 * @brief Clears any existing error state in the interpreter.
 * @param state Interpreter state.
 */
void __clear_error_state(const State* state);

/**
 * @brief Checks whether the interpreter is currently in an error state.
 * @param state Interpreter state.
 * @return true if an error is currently set; false otherwise.
 */
bool __has_error(const State* state);

/**
 * @brief Handles a currently active error by unwinding the call stack.
 * @details Will terminate the program if no handler is found.
 * @param state Interpreter state.
 * @return true if the error was successfully handled; false otherwise.
 */
bool __handle_error(State* state);

/**
 * @brief Retrieves a constant value from the constant pool.
 * @param state Interpreter state.
 * @param index Index of the constant.
 * @return The constant value.
 */
Value __get_constant(const State* state, size_t index);

/**
 * @brief Returns the type of a value as a via string object.
 * @param val The value to inspect.
 * @return Type as a via string Value.
 */
Value __type(const Value& val);

/**
 * @brief Returns the type of a value as a C++ string.
 * @param val The value to inspect.
 * @return Type as std::string.
 */
std::string __type_cxx(const Value& val);

/**
 * @brief Gets the raw pointer stored in a value, or NULL if not applicable.
 * @param val The value to inspect.
 * @return The underlying pointer.
 */
void* __to_pointer(const Value& val);

/**
 * @brief Returns the current call frame on the stack.
 * @param state Interpreter state.
 * @return Pointer to the current call frame.
 */
CallFrame* __current_callframe(State* state);

/**
 * @brief Pushes a new call frame onto the call stack.
 * @param state Interpreter state.
 * @param frame Call frame to push.
 */
void __push_callframe(State* state, CallFrame&& frame);

/**
 * @brief Pops the topmost call frame from the stack.
 * @param state Interpreter state.
 */
void __pop_callframe(State* state);

/**
 * @brief Calls a function using a dynamic dispatch system.
 * @details Supports native and external function types.
 * @param state Interpreter state.
 * @param callee Function to call.
 */
void __call(State* state, Closure* callee);

/**
 * @brief Calls a function in a protected manner using a dynamic dispatch system.
 * @details Supports native and external function types.
 * @param state Interpreter state.
 * @param callee Function to call.
 */
void __pcall(State* state, Closure* callee);

/**
 * @brief Performs a return from a function.
 * @details Restores the previous call frame and pushes a return value.
 * @param state Interpreter state.
 * @param retv Value to return.
 */
void __return(State* VIA_RESTRICT state, Value&& retv);

/**
 * @brief Returns the length of the given value as a Value object.
 *
 * Supports arrays, strings, and dictionaries. Returns Nil if length cannot be determined.
 *
 * @param val The value to get the length of.
 * @return Value Length as an integer Value or Nil.
 */
Value __length(const Value& val);

/**
 * @brief Returns the length of the given value as a C++ integer.
 *
 * Returns -1 if the value has no meaningful length (e.g., number, bool, nil).
 *
 * @param val The value to get the length of.
 * @return int Length or -1 on failure.
 */
int __length_cxx(const Value& val);

/**
 * @brief Converts the given value to a language-level String object.
 *
 * Uses the value's stringification rules defined by the VM or language runtime.
 *
 * @param val The value to convert.
 * @return Value The value as a String object.
 */
Value __to_string(const Value& val);

/**
 * @brief Converts the given value to a C++ `std::string`.
 *
 * Performs the same logic as `__to_string`, but returns a native string.
 *
 * @param val The value to convert.
 * @return std::string The stringified value.
 */
std::string __to_cxx_string(const Value& val);

/**
 * @brief Converts the value to a literal string without applying any escaping or transformation.
 *
 * Useful for printing raw string content.
 *
 * @param val The value to convert.
 * @return std::string The raw literal string.
 */
std::string __to_literal_cxx_string(const Value& val);

/**
 * @brief Converts a value to its boolean representation.
 *
 * Applies standard truthiness rules:
 * - false and nil are false,
 * - everything else is true.
 *
 * @param val The value to evaluate.
 * @return Value A boolean Value representing the truthiness.
 */
Value __to_bool(const Value& val);

/**
 * @brief Returns the truthiness of a value as a native `bool`.
 *
 * Same logic as `__to_bool`, but returns a C++ boolean.
 *
 * @param val The value to evaluate.
 * @return bool True if value is truthy, false otherwise.
 */
bool __to_cxx_bool(const Value& val);

/**
 * @brief Converts the given value to an integer Value or returns Nil if not possible.
 *
 * Attempts string-to-int and float-to-int conversion if applicable.
 *
 * @param V The VM state (used for allocations or error tracking).
 * @param val The value to convert.
 * @return Value Integer Value or Nil.
 */
Value __to_int(const State* V, const Value& val);

/**
 * @brief Converts the given value to a floating-point Value or returns Nil if not possible.
 *
 * Performs conversion from strings or integers as needed.
 *
 * @param V The VM state (used for allocations or error tracking).
 * @param val The value to convert.
 * @return Value Float Value or Nil.
 */
Value __to_float(const State* V, const Value& val);

/**
 * @brief Deeply compares two values for equality.
 *
 * Handles primitive types and recursively compares structures like arrays and dictionaries.
 *
 * @param val0 First value.
 * @param val1 Second value.
 * @return bool True if values are shallow-ly equal, false otherwise.
 */
bool __compare(const Value& val0, const Value& val1);

/**
 * @brief Deeply compares two values for equality.
 *
 * Handles primitive types and recursively compares structures like arrays and dictionaries.
 *
 * @param val0 First value.
 * @param val1 Second value.
 * @return bool True if values are deeply equal, false otherwise.
 */
bool __compare_deep(const Value& val0, const Value& val1);

// Automatically resizes UpValue vector of closure by VIA_UPV_RESIZE_FACTOR.
void __closure_upvs_resize(Closure* closure);

// Checks if a given index is within the bounds of the UpValue vector of the closure.
// Used for resizing.
bool __closure_upvs_range_check(Closure* closure, size_t index);

// Attempts to retrieve UpValue at index <upv_id>.
// Returns nullptr if <upv_id> is out of UpValue vector bounds.
UpValue* __closure_upv_get(Closure* closure, size_t upv_id);

/**
 * @brief Resizes the UpValue vector of the given closure.
 * @note The vector is grown by a constant `VIA_UPV_RESIZE_FACTOR`.
 * @param closure Pointer to the closure.
 */
void __closure_upvs_resize(Closure* closure);

/**
 * @brief Checks if the given index is within bounds of the closure's UpValue vector.
 *
 * @param closure Pointer to the closure.
 * @param index Index to validate.
 * @return bool True if within range, false otherwise.
 */
bool __closure_upvs_range_check(Closure* closure, size_t index);

/**
 * @brief Gets the UpValue at the specified index.
 *
 * @param closure Pointer to the closure.
 * @param upv_id Index of the UpValue.
 * @return UpValue* Pointer to UpValue or nullptr if out of bounds.
 */
UpValue* __closure_upv_get(Closure* closure, size_t upv_id);

/**
 * @brief Sets the UpValue at the specified index to a given value.
 *
 * @param closure Pointer to the closure.
 * @param upv_id Index of the UpValue.
 * @param val Value to set.
 */
void __closure_upv_set(Closure* closure, size_t upv_id, Value& val);

/**
 * @brief Loads bytecode instructions into the closure.
 *
 * Handles special opcodes like RET and CAPTURE during loading.
 *
 * @param state Runtime state.
 * @param closure Target closure.
 * @param len Length of bytecode stream.
 */
void __closure_init(State* state, Closure* closure, size_t len);

/**
 * @brief Closes the closure’s upvalues and moves them to the heap.
 *
 * Typically used at function return to preserve captured variables.
 *
 * @param closure The closure whose upvalues to close.
 */
void __closure_close_upvalues(const Closure* closure);

/**
 * @brief Hashes a key string using FNV-1a.
 *
 * @param dict Dictionary context.
 * @param key Null-terminated string key.
 * @return size_t Hash value.
 */
size_t __dict_hash_key(const Dict* dict, const char* key);

/**
 * @brief Sets a key-value pair in the dictionary.
 *
 * Updates existing key if already present.
 *
 * @param dict Pointer to dictionary.
 * @param key Null-terminated key.
 * @param val Value to insert.
 */
void __dict_set(const Dict* dict, const char* key, Value val);

/**
 * @brief Retrieves the value associated with a key.
 *
 * @param dict Pointer to dictionary.
 * @param key Null-terminated key string.
 * @return Value* Pointer to value or nullptr if key not found.
 */
Value* __dict_get(const Dict* dict, const char* key);

/**
 * @brief Returns number of entries in the dictionary.
 *
 * @param dict Pointer to dictionary.
 * @return size_t Number of key-value pairs.
 */
size_t __dict_size(const Dict* dict);

/**
 * @brief Checks if an index is valid in the array.
 *
 * @param array Pointer to array.
 * @param index Index to check.
 * @return bool True if valid, false otherwise.
 */
bool __array_range_check(const Array* array, size_t index);

/**
 * @brief Resizes the array component of the table.
 *
 * Grows capacity to accommodate more elements.
 *
 * @param array Pointer to array.
 */
void __array_resize(Array* array);

/**
 * @brief Sets a value at a specific index in the array.
 *
 * Resizes if the index exceeds current capacity.
 *
 * @param array Pointer to array.
 * @param index Index to assign.
 * @param val Value to assign.
 */
void __array_set(Array* array, size_t index, Value val);

/**
 * @brief Retrieves a value at a specific index.
 *
 * @param array Pointer to array.
 * @param index Index to read.
 * @return Value* Pointer to value or nullptr if out of bounds.
 */
Value* __array_get(const Array* array, size_t index);

/**
 * @brief Returns the current size (number of used elements) of the array.
 *
 * @param array Pointer to array.
 * @return size_t Array size.
 */
size_t __array_size(const Array* array);

/**
 * @brief Allocates space for a set number of labels in the state.
 *
 * @param state The runtime state.
 * @param count Number of labels to allocate.
 */
void __label_allocate(State* state, size_t count);

/**
 * @brief Deallocates label memory in the given state.
 *
 * @param state The runtime state.
 */
void __label_deallocate(State* state);

/**
 * @brief Returns a pointer to a label instruction by index.
 *
 * @param state The runtime state.
 * @param index Index of label.
 * @return Instruction* Pointer to instruction or nullptr.
 */
Instruction* __label_get(const State* state, size_t index);

/**
 * @brief Loads the label instruction table into the state.
 *
 * @param state The runtime state.
 */
void __label_load(const State* state);

/**
 * @brief Pushes a value onto the VM stack.
 *
 * @param state The runtime state.
 * @param val Value to push.
 */
void __push(State* state, Value&& val);

/**
 * @brief Drops the top value from the VM stack.
 *
 * @param state The runtime state.
 */
void __drop(State* state);

/**
 * @brief Retrieves a local variable at a given offset.
 *
 * @param state The runtime state.
 * @param offset Stack frame offset.
 * @return Value* Pointer to local value.
 */
Value* __get_local(State* VIA_RESTRICT state, size_t offset);

/**
 * @brief Sets a local variable at a given offset.
 *
 * @param state The runtime state.
 * @param offset Stack frame offset.
 * @param val Value to set.
 */
void __set_local(State* VIA_RESTRICT state, size_t offset, Value&& val);

/**
 * @brief Allocates the VM register table.
 *
 * @param state The runtime state.
 */
void __register_allocate(State* state);

/**
 * @brief Frees the register table.
 *
 * @param state The runtime state.
 */
void __register_deallocate(const State* state);

/**
 * @brief Assigns a value to a register.
 *
 * @param state The runtime state.
 * @param reg Register index.
 * @param val Value to assign.
 */
void __set_register(const State* state, operand_t reg, Value&& val);

/**
 * @brief Retrieves a value from a register.
 *
 * @param state The runtime state.
 * @param reg Register index.
 * @return Value* Pointer to register value.
 */
Value* __get_register(const State* state, operand_t reg);

/**
 * @brief Creates the main function closure for a compilation unit.
 * @param lctx Translation unit context.
 * @return Pointer to main closure.
 */
Closure* __create_main_function(Context& lctx);

/**
 * @brief Declares the built-in core library into the interpreter state.
 * @param state Interpreter state.
 */
void __declare_core_lib(State* state);

} // namespace impl

/** @} */

} // namespace via

/** @} */

#endif
