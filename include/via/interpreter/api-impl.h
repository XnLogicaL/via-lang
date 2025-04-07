//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_VMAPI_H
#define VIA_HAS_HEADER_VMAPI_H

#include "common.h"
#include "constant.h"
#include "instruction.h"
#include "opcode.h"
#include "state.h"
#include "string-utility.h"
#include "api-aux.h"

namespace via::impl {

// Internal function for throwing errors.
void __set_error_state(state* state, const std::string& message);

// Internal function for discarding errors.
void __clear_error_state(state* state);

// Internal function that returns whether if an error has been thrown but not handled.
bool __has_error(state* state);

/**
 * Internal function that handles errors when thrown. Unwinds the stack until a stack-frame with
 * error-handling capabilities is found. If the main stack-frame is touched, automatically
 * terminates the program.
 */
bool __handle_error(state* state);

// Internal function used to get the kvalue from the constant table at the given index.
value_obj __get_constant(state* state, size_t index);

// Internal function that returns the type of the given object as a string.
value_obj __type(const value_obj& val);

// Internal function that returns the type of the given object as an std::string.
std::string __type_cxx(const value_obj& val);

/**
 * Internal function that returns the underlying pointer of the value if the value type is
 * heap-allocated, nullptr if not.
 */
void* __to_pointer(const value_obj& val);

/**
 * Internal function that calls the given native function with a given argument count.
 * Sets up the callstack and callframe for the given function after injecting call_info into it.
 */
void __native_call(state* VIA_RESTRICT state, function_obj* VIA_RESTRICT _Callee, size_t _Argc);

/**
 * Internal function used to call C function pointers with a given argument count.
 * Emulates the call like a native function, setting up the callstack and callframe exactly how it
 * would if the C function were to be interpreted as a native function.
 */
void __extern_call(state* VIA_RESTRICT state, const value_obj& _Callee, size_t _Argc);

/**
 * Internal function that serves as a generalized call interface. Used to call any type.
 * Invokes `__native_call` for native functions, `__extern_call` for C functions.
 */
void __call(state* state, value_obj& _Callee, size_t _Argc);

// Internal function that returns the length of the given value or nil if impossible.
value_obj __length(const value_obj& val);

// Internal function that returns the length of the given value as a int or -1 if impossible.
int __length_cxx(const value_obj& val);

/**
 * Internal function that performs a return operation. Restores the stack pointer and program
 * counter to the top-most callframes call_data. After the stack pointer is restored, pushes a
 * return value back onto the stack.
 */
void __native_return(state* VIA_RESTRICT state, value_obj _Ret_value);

// Internal function that reutrns the given value as a string object.
value_obj __to_string(const value_obj& val);

// Internal function that returns the given value as an std::string.
std::string __to_cxx_string(const value_obj& val);

// Internal function that returns the given value as a literal std::string.
std::string __to_literal_cxx_string(const value_obj& val);

// Internal function that returns the truthy-ness of the given value.
value_obj __to_bool(const value_obj& val);

// Internal function that returns the truthy-ness of the given value as a boolean.
bool __to_cxx_bool(const value_obj& val);

// Internal function that returns the int representation of the given value or nil if impossible.
value_obj __to_int(state* V, const value_obj& val);

// Internal function that returns the FP representation of the given value or nil if impossible.
value_obj __to_float(state* V, const value_obj& val);

// Internal function that performs a deep equality comparison between two given values.
bool __compare(const value_obj& val0, const value_obj& val1);

} // namespace via::impl

#endif
