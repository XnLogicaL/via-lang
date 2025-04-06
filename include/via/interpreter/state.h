// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_STATE_H
#define VIA_HAS_HEADER_STATE_H

#include "common.h"
#include "instruction.h"
#include "object.h"

#define VIA_VMSTACKSIZE    2048
#define VIA_STK_REGISTERS  256
#define VIA_HEAP_REGISTERS 65536 - VIA_STK_REGISTERS
#define VIA_ALL_REGISTERS  VIA_STK_REGISTERS + VIA_HEAP_REGISTERS

namespace via {

// Forward declarations
struct function_obj;

// Calling convention
enum class call_type {
  NOCALL,
  CALL,
  FASTCALL,
};

struct error_state {
  function_obj* frame = nullptr;
  std::string message = "";
};

// Global state, should only be instantiated once, and shared across all worker contexts.
struct global_state {
  std::unordered_map<uint32_t, string_obj*> stable; // String interning table
  std::atomic<uint32_t> threads{0};                 // Thread count
  table_obj gtable;                                 // global_obj environment

  std::shared_mutex stable_mutex;
};

// Register array wrapper.
template<const size_t Size>
struct alignas(64) register_holder {
  value_obj registers[Size];
};

// Type aliases for stack registers and heap registers.
using stack_registers_t = register_holder<VIA_STK_REGISTERS>;
using spill_registers_t = register_holder<VIA_HEAP_REGISTERS>;

// "Per worker" execution context. Manages things like registers, stack, heap of the VM thread.
// 64-byte alignment for maximum cache friendliness.
struct alignas(64) state {
  // Thread and global state
  uint32_t id;       // Thread ID
  global_state* glb; // global_obj state

  // instruction pointers
  instruction* pc = nullptr;   // Current instruction pointer
  instruction* ibp = nullptr;  // Instruction buffer pointer
  instruction* sibp = nullptr; // Saved instruction buffer pointer

  // Stack state
  size_t sp = 0;  // Stack pointer
  value_obj* sbp; // Stack base pointer

  // Labels
  instruction** labels; // Label array

  // Call and frame management
  function_obj* frame = nullptr; // Call stack pointer

  // VM control and debugging
  bool abort = false;
  error_state* err;

  // Register holders
  stack_registers_t& stack_registers;
  spill_registers_t* spill_registers;

  // Translation unit context reference
  trans_unit_context& unit_ctx;

  // ===========================================================================================
  // Meta
public:
  // Make uncopyable
  VIA_NOCOPY(state);

  // Make movable
  VIA_IMPLMOVE(state);

  // Constructor
  explicit state(
    global_state* global, stack_registers_t& stk_registers, trans_unit_context& unit_ctx
  );

  // Destructor
  ~state();

  // Loads the given containers data into the instruction buffer.
  void load(const bytecode_holder& bytecode);

  // ===========================================================================================
  // Execution flow

  // Starts thread execution.
  void execute();

  // ===========================================================================================
  // Register manipulation

  // Returns a reference to the value that lives in a given register.
  value_obj& get_register(operand_t reg);

  // Sets a given register to a given value.
  void set_register(operand_t reg, value_obj value);

  // ===========================================================================================
  // Comparison and metadata

  // Returns whether if a given value has a heap-allocated component.
  bool is_heap(const value_obj& value);

  // Compares two given values.
  bool compare(const value_obj& left, const value_obj& right);

  // ===========================================================================================
  // Basic stack manipulation

  // Pushes nil onto the stack.
  void push_nil();

  // Pushes an integer onto the stack.
  void push_int(TInteger value);

  // Pushes a float onto the stack.
  void push_float(TFloat value);

  // Pushes a boolean with value `true` onto the stack.
  void push_true();

  // Pushes a boolean with value `false` onto the stack.
  void push_false();

  // Pushes a string onto the stack.
  void push_string(const char* str);

  // Pushes an empty table onto the stack.
  void push_table();

  // Pushes a value onto the stack.
  void push(value_obj value);

  // Drops a value from the stack, frees the resources of the dropped value.
  void drop();

  // Pops a value from the stack and returns it.
  value_obj pop();

  // Returns the top value on the stack.
  const value_obj& top();

  // ===========================================================================================
  // Advanced stack manipulation

  // Sets the value at a given position on the stack to a given value.
  void set_stack(size_t position, value_obj value);

  // Returns the stack value at a given position.
  const value_obj& get_stack(size_t position);

  // Returns the stack value at a given offset relative to the current stack-frame's stack
  // pointer.
  const value_obj& get_argument(size_t offset);

  // Returns the size of stack.
  size_t stack_size();

  // ===========================================================================================
  // global_obj manipulation

  // Returns the global that corresponds to a given hashed identifier.
  value_obj get_global(const char* name);

  // Sets the global that corresponds to a given hashed identifier to a given value.
  void set_global(const char* name, const value_obj& value);

  // ===========================================================================================
  // Function manipulation

  // Standard return. Returns from the current function with an optional value.
  void native_return(const value_obj& return_value);

  // Calls the given function_obj object with a given argument count.
  void native_call(function_obj* target, size_t argc);

  // Calls the method that lives in a given index of a given object with a given argument count.
  void method_call(object_obj* object, size_t index, size_t argc);

  // Attempts to call the given value object with the given argument count.
  void call(const value_obj& callee, size_t argc);

  // Attempts to return the upv_obj that lives in the given index of the given closure.
  const value_obj& get_upvalue(function_obj* closure, size_t index);

  // Attempts to set the upv_obj that lives in the given index of the given closure to a given
  // value.
  void set_upvalue(function_obj* closure, size_t index, const value_obj& value);

  // Returns the upv_obj count of the given closure.
  size_t get_upvalue_count(function_obj* closure);

  // Returns the local count of the given closure.
  size_t get_local_count_closure(function_obj* closure);

  // ===========================================================================================
  // General operations
};

} // namespace via

#endif
