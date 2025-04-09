//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_STATE_H
#define VIA_HAS_HEADER_STATE_H

#include "common.h"
#include "instruction.h"
#include "object.h"

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
struct IFunction;

// Calling convention
enum class call_type {
  NOCALL,
  CALL,
  FASTCALL,
};

struct error_state {
  IFunction* frame = nullptr;
  std::string message = "";
};

// Global state, should only be instantiated once, and shared across all worker contexts.
struct global_state {
  std::unordered_map<uint32_t, IString*> stable; // String interning table
  std::atomic<uint32_t> threads{0};              // Thread count
  IDict gtable;                                  // CompilerGlobal environment

  std::shared_mutex stable_mutex;
};

// Register array wrapper.
template<const size_t Size>
struct alignas(64) register_holder {
  IValue registers[Size];
};

// Type aliases for stack registers and heap registers.
using stack_registers_t = register_holder<VIA_STK_REGISTERS>;
using spill_registers_t = register_holder<VIA_HEAP_REGISTERS>;

/**
 * "Per worker" execution context. Manages things like registers, stack, heap of the VM thread.
 * 64-byte aligned for maximum cache friendliness.
 */
struct alignas(64) state {
  // Thread and global state
  uint32_t id;       // Thread ID
  global_state* glb; // CompilerGlobal state

  // instruction pointers
  Instruction* pc = nullptr;   // Current instruction pointer
  Instruction* ibp = nullptr;  // Instruction buffer pointer
  Instruction* sibp = nullptr; // Saved instruction buffer pointer

  // Stack state
  size_t sp = 0; // Stack pointer
  IValue* sbp;   // Stack base pointer

  // Labels
  Instruction** labels; // Label array

  // Call and frame management
  IFunction* frame = nullptr; // Call stack pointer

  // VM control and debugging
  bool abort = false;
  error_state* err;

  // Register holders
  stack_registers_t& stack_registers;
  spill_registers_t* spill_registers;

  // Translation unit context reference
  TransUnitContext& unit_ctx;

  //  ==================
  // [ Object semantics ]
  //  ==================

  // Make uncopyable
  VIA_NOCOPY(state);
  // Make movable
  VIA_IMPLMOVE(state);

  // Constructor
  explicit state(
    global_state* global, stack_registers_t& stk_registers, TransUnitContext& unit_ctx
  );

  // Destructor
  ~state();

  //  ==============
  // [ Core methods ]
  //  ==============

  // Loads the given containers data into the instruction buffer.
  void load(const BytecodeHolder&);

  //  ================
  // [ Execution flow ]
  //  ================

  // Starts thread execution.
  void execute();

  //  =======================
  // [ Register manipulation ]
  //  =======================

  // Returns a reference to the value that lives in a given register.
  IValue& get_register(operand_t reg);

  // Sets a given register to a given value.
  void set_register(operand_t reg, IValue value);

  //  ==========================
  // [ Basic stack manipulation ]
  //  ==========================

  // Pushes nil onto the stack.
  void push_nil();

  // Pushes an integer onto the stack.
  void push_int(int value);

  // Pushes a float onto the stack.
  void push_float(float value);

  // Pushes a boolean with value `true` onto the stack.
  void push_true();

  // Pushes a boolean with value `false` onto the stack.
  void push_false();

  // Pushes a string onto the stack.
  void push_string(const char* str);

  // Pushes an empty array onto the stack.
  void push_array();

  // Pushes an empty dictionary onto the stack.
  void push_dict();

  // Pushes a value onto the stack.
  void push(IValue value);

  // Drops a value from the stack, frees the resources of the dropped value.
  void drop();

  // Pops a value from the stack and returns it.
  IValue pop();

  // Returns the top value on the stack.
  const IValue& top();

  //  =============================
  // [ Advanced stack manipulation ]
  //  =============================

  // Sets the value at a given position on the stack to a given value.
  void set_stack(size_t position, IValue value);

  // Returns the stack value at a given position.
  IValue& get_stack(size_t position);

  // Returns the stack value at a given offset relative to the current stack-frame's stack
  // pointer.
  IValue& get_argument(size_t offset);

  // Returns the size of stack.
  size_t stack_size();

  //  =====================
  // [ Global manipulation ]
  //  =====================

  // Returns the global that corresponds to a given hashed identifier.
  IValue& get_global(const char* name);

  // Sets the global that corresponds to a given hashed identifier to a given value.
  void set_global(const char* name, const IValue& value);

  //  =======================
  // [ IFunction manipulation ]
  //  =======================

  // Standard return. Returns from the current function with an optional value.
  void native_return(const IValue& return_value);

  // Calls the given IFunction object with a given argument count.
  void native_call(IFunction& target, size_t argc);

  // Calls the method that lives in a given index of a given object with a given argument count.
  void method_call(IObject& object, size_t index, size_t argc);

  // Attempts to call the given value object with the given argument count.
  void call(const IValue& callee, size_t argc);

  // ===========================================================================================
  // General operations
};

} // namespace via

#endif
