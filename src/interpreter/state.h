// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file state.h
 * @brief Declares the State class and related components for managing VM execution.
 *
 * The State object encapsulates the full runtime state of the virtual machine.
 * It holds the register space (stack and heap), manages the call stack, and controls
 * instruction dispatch and execution. It provides an interface for interacting with
 * local, global, and argument values, as well as for calling functions and handling errors.
 */
#ifndef VIA_HAS_HEADER_STATE_H
#define VIA_HAS_HEADER_STATE_H

#include <common.h>
#include <context.h>
#include "call-stack.h"
#include "instruction.h"
#include "tvalue.h"

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @brief Total number of general-purpose registers available.
 * Matches the operand range (2^16).
 */
inline constexpr size_t REGISTER_COUNT = 65536;

/**
 * @brief Number of registers reserved for stack-based usage.
 * These are backed by fast C++ stack memory.
 */
inline constexpr size_t REGISTER_STACK_COUNT = 256;

/**
 * @brief Number of registers reserved for heap-based spilling.
 * These are allocated separately and used when stack registers are exhausted.
 */
inline constexpr size_t REGISTER_SPILL_COUNT = REGISTER_COUNT - REGISTER_STACK_COUNT;

/**
 * @struct ErrorState
 * @brief Represents an active runtime error during VM execution.
 */
struct ErrorState {
  CallFrame* frame = nullptr; ///< Call frame where the error occurred.
  std::string message = "";   ///< Human-readable error message.
};

/**
 * @brief Generic register array wrapper with fixed size and alignment.
 * @tparam Size The number of registers in the holder.
 */
template<const size_t Size>
struct alignas(64) RegisterHolder {
  Value registers[Size];
};

/// Type alias for stack-based register block.
using StkRegHolder = RegisterHolder<REGISTER_STACK_COUNT>;

/// Type alias for heap/spill register block.
using HeapRegHolder = RegisterHolder<REGISTER_SPILL_COUNT>;

/**
 * @class State
 * @brief Represents the complete virtual machine execution state.
 *
 * This object owns and manages the program counter, call stack, register file,
 * globals, error reporting, and runtime execution loop. The structure is aligned
 * to 64 bytes to ensure optimal CPU cache usage during high-frequency access.
 */
struct alignas(64) State {
public:
  VIA_NOCOPY(State);
  VIA_NOMOVE(State);

  /**
   * @brief Constructs a new State.
   * @param stack_regs Reference to the stack register block.
   * @param unit_ctx Reference to the translation unit's compilation context.
   */
  explicit State(StkRegHolder& stack_regs, TransUnitContext& unit_ctx);
  ~State();

  /// Begins executing instructions starting at the current program counter (`pc`).
  void execute();

  /// Executes a single instruction step. Optionally provide an override instruction.
  void execute_step(std::optional<Instruction> insn = std::nullopt);

  /// Returns a mutable reference to the value stored in the given register.
  Value& get_register(operand_t reg);

  /// Assigns a value to the given register.
  void set_register(operand_t reg, Value value);

  /// Pushes a `Nil` value onto the stack.
  void push_nil();

  /// Pushes an integer value onto the stack.
  void push_int(int value);

  /// Pushes a floating-point value onto the stack.
  void push_float(float value);

  /// Pushes a boolean `true` onto the stack.
  void push_true();

  /// Pushes a boolean `false` onto the stack.
  void push_false();

  /// Pushes a string value onto the stack.
  void push_string(const char* str);

  /// Pushes an empty array onto the stack.
  void push_array();

  /// Pushes an empty dictionary onto the stack.
  void push_dict();

  /// Pushes a generic value onto the stack.
  void push(Value value);

  /// Drops the top value from the stack and frees its resources.
  void drop();

  /// Assigns a value to a local variable at the given position.
  void set_local(size_t position, Value value);

  /// Returns a reference to a local value at the given position.
  Value& get_local(size_t position);

  /// Returns a reference to an argument value relative to the current call frame.
  Value& get_argument(size_t offset);

  /// Returns the current size of the value stack.
  size_t stack_size();

  /// Retrieves the global variable with the given name.
  Value& get_global(const char* name);

  /// Assigns a value to a global variable with the given name.
  void set_global(const char* name, const Value& value);

  /// Invokes a function closure with a given number of arguments.
  void call(const Closure& callee, size_t argc);

public:
  Instruction* pc = nullptr;      ///< Current instruction pointer.
  Instruction** labels;           ///< Jump target label array.
  Dict* globals;                  ///< Global variable dictionary.
  CallStack* callstack;           ///< Call stack of function frames.
  ErrorState* err;                ///< Active error state, if any.
  Value main;                     ///< Reference to the main function.
  register_t ret;                 ///< Return register index.
  register_t args;                ///< First argument register index.
  StkRegHolder& stack_registers;  ///< Stack register block.
  HeapRegHolder* spill_registers; ///< Heap (spill) register block.
  TransUnitContext& unit_ctx;     ///< Translation unit compilation context.
};

} // namespace via

/** @} */

#endif
