// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file callstack.h
 * @brief Defines the call stack and call frame structures for function execution.
 * @details The call stack is a runtime structure that manages active function calls during
 * the execution of programs. Each function call is represented by a `CallFrame`,
 * which stores the execution context for that function, including its closure,
 * local variables, and return address. The `CallStack` holds a fixed number of
 * frames, ensuring bounded recursion and stack safety.
 */
#ifndef VIA_HAS_HEADER_CALL_STACK_H
#define VIA_HAS_HEADER_CALL_STACK_H

#include "common.h"
#include "tfunction.h"

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @brief Maximum number of frames that the call stack can hold at any time.
 * @details This value places a fixed limit on the recursion depth and number of active function
 * calls.
 */
inline constexpr size_t CALLSTACK_MAX_FRAMES = 200;

/**
 * @brief Maximum number of local variables a single call frame can support.
 * @dtails Used primarily as a safety bound and for fixed-size local storage per function.
 */
inline constexpr size_t CALLFRAME_MAX_LOCALS = 200;

/**
 * @struct CallFrame
 * @brief Represents a single function invocation's execution context.
 *
 * Each `CallFrame` holds:
 * - A pointer to the `Closure` being executed (compiled function and captured upvalues).
 * - A pointer to a local variables array (`locals`).
 * - The size of the local variables array (`locals_size`).
 * - A saved program counter (`savedpc`) indicating where execution resumes after return.
 *
 * Copy operations are disabled via `VIA_NOCOPY`, but move semantics are allowed via `VIA_IMPLMOVE`.
 */
struct CallFrame {
  bool is_protected = false;
  Closure* closure = nullptr;     ///< Function closure being invoked.
  Value* locals = nullptr;        ///< Pointer to local variable storage.
  size_t locals_size = 0;         ///< Number of allocated local variables.
  Instruction* savedpc = nullptr; ///< Instruction pointer saved for return.

  VIA_NOCOPY(CallFrame);   ///< Disable copy constructor and copy assignment.
  VIA_IMPLMOVE(CallFrame); ///< Enable move constructor and move assignment.

  CallFrame();
  ~CallFrame();
};

/**
 * @struct CallStack
 * @brief Represents the function call stack of the interpreter.
 *
 * Contains a fixed-size array of `CallFrame` structures and a count of the current depth.
 * Used to manage the execution of nested or recursive function calls.
 */
struct CallStack {
  size_t frames_count = 0;                ///< Number of currently active frames.
  CallFrame frames[CALLSTACK_MAX_FRAMES]; ///< Stack-allocated array of call frames.
};

} // namespace via

/** @} */

#endif
