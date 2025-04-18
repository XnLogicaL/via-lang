// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_STATE_H
#define VIA_HAS_HEADER_STATE_H

#include "call-stack.h"
#include "common.h"
#include "instruction.h"
#include "tvalue.h"

// Maximum amount of objects on the virtual stack.
#define VIA_VMSTACKSIZE 2048
// Stack-allocated "hot" register count.
#define VIA_STK_REGISTERS 256
// Heap-allocated "spill" register count.
#define VIA_HEAP_REGISTERS 65536 - VIA_STK_REGISTERS
// Combined stack + heap allocated register count.
#define VIA_ALL_REGISTERS VIA_STK_REGISTERS + VIA_HEAP_REGISTERS

//  =========
// [ state.h ]
//  =========
namespace via {

// Forward declarations
struct Closure;

struct ErrorState {
  CallFrame* frame = nullptr;
  std::string message = "";
};

// Global state, should only be instantiated once, and shared across all worker contexts.
struct GlobalState {
  std::unordered_map<uint32_t, String*> stable; // String interning table
  std::atomic<uint32_t> threads{0};             // Thread count
  Dict* gtable;                                 // CompilerGlobal environment

  std::shared_mutex stable_mutex;
};

// Register array wrapper.
template<const size_t Size>
struct alignas(64) RegisterHolder {
  Value registers[Size];
};

// Type aliases for stack registers and heap registers.
using StkRegHolder = RegisterHolder<VIA_STK_REGISTERS>;
using HeapRegHolder = RegisterHolder<VIA_HEAP_REGISTERS>;

/**
 * "Per worker" execution context. Manages things like registers, stack, heap of the VM thread.
 * 64-byte aligned for maximum cache friendliness.
 */
struct alignas(64) State {
public:
  VIA_NOCOPY(State);
  VIA_NOMOVE(State);

  explicit State(GlobalState&, StkRegHolder&, TransUnitContext&);
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
  uint32_t id;      // Thread ID
  GlobalState& glb; // CompilerGlobal state

  Instruction* pc = nullptr; // Current instruction pointer
  Instruction** labels;      // Label array

  CallStack* callstack; // Call stack
  ErrorState* err;      // Error state
  Value main;           // Main function
  Value ret;            // Return value slot

  StkRegHolder& stack_registers;  // Stack register holder
  HeapRegHolder* spill_registers; // Spill register holder

  TransUnitContext& unit_ctx; // Translation unit context reference
};

} // namespace via

#endif
