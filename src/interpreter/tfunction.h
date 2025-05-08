// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file tfunction.h
 * @brief Declares function, closure, and upvalue types used for virtual machine function
 * invocation.
 *
 * This includes both user-defined and native function representations, along with closures and
 * upvalue capture logic for supporting lexical scoping and first-class functions.
 */
#ifndef VIA_HAS_HEADER_FUNCTION_H
#define VIA_HAS_HEADER_FUNCTION_H

#include "common.h"
#include "instruction.h"
#include "tvalue.h"

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @brief Default number of upvalues reserved during closure initialization.
 */
inline constexpr size_t CLOSURE_INITIAL_UPV_COUNT = 10;

struct State;
struct Closure;
struct CallFrame;

/**
 * @struct UpValue
 * @brief Represents a captured variable in a closure.
 *
 * An UpValue can either point directly to a value still on the stack (open),
 * or contain a heap-allocated copy of the value (closed).
 */
struct UpValue {
  bool is_open = true;        ///< Whether the upvalue is open (points to stack).
  bool is_valid = false;      ///< Whether the upvalue has been properly initialized.
  Value* value = nullptr;     ///< Pointer to the actual value, or null.
  Value heap_value = Value(); ///< Used to store the value when closed.
};

/**
 * @struct Function
 * @brief Represents a user-defined via function, including its bytecode and metadata.
 */
struct Function {
  Instruction* code = nullptr;    ///< Pointer to the function’s instruction sequence.
  size_t code_size = 0;           ///< Total number of instructions.
  size_t line_number = 0;         ///< Line number where function was defined (for debugging).
  const char* id = "<anonymous>"; ///< Identifier string or default name.
};

/**
 * @brief Type alias for native C++ functions that can be called by the VM.
 * They receive a pointer to the current interpreter state and return a `Value`.
 */
using NativeFn = Value (*)(State* interpreter);

/**
 * @struct Callable
 * @brief Wraps a function-like object, either user-defined or native.
 *
 * Used uniformly throughout the VM for calling both compiled and native routines.
 */
struct Callable {
  /**
   * @enum Tag
   * @brief Indicates the kind of function this Callable represents.
   */
  enum class Tag {
    None,     ///< No function.
    Function, ///< User-defined function.
    Native,   ///< Native function.
  } type = Tag::Function;

  /**
   * @union Un
   * @brief Stores either a pointer to a `Function` or a `NativeFn`.
   */
  union Un {
    Function fn;
    NativeFn ntv;
  } u = {.ntv = nullptr};

  size_t arity = 0;              ///< Number of arguments expected.
  bool is_error_handler = false; ///< Whether if the function is an error handler.

  Callable() = default;
};

/**
 * @struct Closure
 * @brief Wraps a Callable with its captured upvalues for lexical scoping.
 *
 * A Closure is created when a function expression references non-local variables.
 */
struct Closure {
  Callable callee;         ///< Underlying callable (function or native).
  UpValue* upvs = nullptr; ///< Array of upvalue pointers.
  size_t upv_count = 0;    ///< Number of captured upvalues.

  VIA_IMPLCOPY(Closure);
  VIA_IMPLMOVE(Closure);

  Closure();

  /**
   * @brief Constructs a closure from a callable (usually a function).
   * @param callable The callable to close over.
   */
  Closure(Callable&& callable);

  /**
   * @brief Destructor that frees upvalue array.
   */
  ~Closure();
};

} // namespace via

/** @} */

#endif
