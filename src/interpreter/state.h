// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_STATE_H
#define VIA_HAS_HEADER_STATE_H

#include "common.h"
#include "context.h"
#include "call-stack.h"
#include "instruction.h"
#include "tvalue.h"

//  =========
// [ state.h ]
//  =========
namespace via {

inline constexpr size_t REGISTER_COUNT = 65536;     // 2^16, operand range
inline constexpr size_t REGISTER_STACK_COUNT = 256; // C stack
inline constexpr size_t REGISTER_SPILL_COUNT = REGISTER_COUNT - REGISTER_STACK_COUNT; // Heap

// Forward declarations
struct Closure;

struct ErrorState {
  CallFrame* frame = nullptr;
  std::string message = "";
};

// Register array wrapper.
template<const size_t Size>
struct alignas(64) RegisterHolder {
  Value registers[Size];
};

// Type aliases for stack registers and heap registers.
using StkRegHolder = RegisterHolder<REGISTER_STACK_COUNT>;
using HeapRegHolder = RegisterHolder<REGISTER_SPILL_COUNT>;

/**
 * Interpreter state object. Manages things like registers, stack, heap of the VM.
 * 64-byte aligned for maximum cache friendliness.
 */
struct alignas(64) State {
public:
  VIA_NOCOPY(State);
  VIA_NOMOVE(State);

  explicit State(StkRegHolder&, TransUnitContext&);
  ~State();

  void execute();
  void execute_step(std::optional<Instruction> insn = std::nullopt);

  // Returns a reference to the value that lives in a given register.
  Value& get_register(operand_t reg);

  // Sets a given register to a given value.
  void set_register(operand_t reg, Value value);

  // Pushes Nil onto the stack.
  void push_nil();

  // Pushes an Int onto the stack.
  void push_int(int value);

  // Pushes a float onto the stack.
  void push_float(float value);

  // Pushes a Bool with value `true` onto the stack.
  void push_true();

  // Pushes a Bool with value `false` onto the stack.
  void push_false();

  // Pushes a String onto the stack.
  void push_string(const char* str);

  // Pushes an empty array onto the stack.
  void push_array();

  // Pushes an empty dictionary onto the stack.
  void push_dict();

  // Pushes a value onto the stack.
  void push(Value value);

  // Drops a value from the stack, frees the resources of the dropped value.
  void drop();

  // Sets the value at a given position on the stack to a given value.
  void set_local(size_t position, Value value);

  // Returns the stack value at a given position.
  Value& get_local(size_t position);

  // Returns the stack value at a given offset relative to the current stack-frame's stack
  // pointer.
  Value& get_argument(size_t offset);

  // Returns the size of stack.
  size_t stack_size();

  // Returns the global that corresponds to a given hashed identifier.
  Value& get_global(const char* name);

  // Sets the global that corresponds to a given hashed identifier to a given value.
  void set_global(const char* name, const Value& value);

  // Attempts to call the given value object with the given argument count.
  void call(const Closure& callee, size_t argc);

public:
  Instruction* pc = nullptr; // Current instruction pointer
  Instruction** labels;      // Label array

  Dict* globals;        // Global table
  CallStack* callstack; // Call stack
  ErrorState* err;      // Error state
  Value main;           // Main function slot
  register_t ret;       // Return register
  register_t args;      // First argument register

  StkRegHolder& stack_registers;  // Stack register holder
  HeapRegHolder* spill_registers; // Spill register holder

  TransUnitContext& unit_ctx; // Translation unit context reference
};

} // namespace via

#endif
