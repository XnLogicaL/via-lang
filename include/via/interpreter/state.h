// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _vl_state_h
#define _vl_state_h

#include "common.h"
#include "instruction.h"
#include "object.h"
#include "signal.h"

#define vl_vmstacksize 2048
#define vl_regcount    0xFFFF

namespace via {

// Forward declarations
struct tfunction;
struct GarbageCollector;

// Calling convention
enum class CallType {
  NOCALL,
  CALL,
  FASTCALL,
};

// State of an State (thread) execution
enum class ThreadState {
  RUNNING,
  PAUSED,
  DEAD,
};

struct ErrorState {
  tfunction* frame = nullptr;
  std::string message = "";
};

// global_obj state, should only be instantiated once, and shared across all
// State's. (threads)
struct GState {
  std::unordered_map<uint32_t, string_obj*> stable; // String interning table
  std::unordered_map<uint32_t, value_obj> gtable;   // global_obj environment
  std::atomic<uint32_t> threads{0};                 // Thread count

  std::shared_mutex stable_mutex;
  std::mutex gtable_mutex;
  std::mutex symtable_mutex;
};

struct alignas(64) State {
  // Thread and global state
  uint32_t id; // Thread ID
  GState* G;   // global_obj state

  // instruction pointers
  instruction* pc = nullptr;  // Current instruction pointer
  instruction* ibp = nullptr; // instruction list begin pointer
  instruction* iep = nullptr; // instruction list end pointer
  instruction* sibp = nullptr;
  instruction* siep = nullptr;

  // Stack state
  value_obj* sbp; // Stack base pointer
  size_t sp = 0;  // Stack pointer

  // Registers
  value_obj* registers;

  // Labels
  instruction** labels;

  // Call and frame management
  tfunction* frame = nullptr; // Call stack pointer

  // VM control and debugging
  bool abort = false;
  ErrorState* err;

  // Thread state
  ThreadState tstate = ThreadState::PAUSED; // Current thread state

  // Signals
  utils::signal<> sig_exit;
  utils::signal<> sig_abort;
  utils::signal<> sig_error;
  utils::signal<> sig_fatal;

  trans_unit_context& unit_ctx;

  vl_nocopy(State);

  State(GState* global, trans_unit_context& unit_ctx);
  ~State();

  void load(const bytecode_holder& bytecode);

  // ===========================================================================================
  // Execution flow

  // Starts thread execution.
  void execute();

  // Pauses thread.
  void pause();

  // Kills the thread indefinitely.
  void kill();

  // ===========================================================================================
  // Register manipulation

  // Returns a reference to the value that lives in a given register.
  value_obj& get_register(operand_t reg);

  // Sets a given register to a given value.
  void set_register(operand_t reg, const value_obj& value);

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
  void push(const value_obj& value);

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
  // Value manipulation

  // Attempts to convert a given value into an integer.
  value_obj to_integer(const value_obj& value);

  // Attempts to convert a given value into a float.
  value_obj to_float(const value_obj& value);

  // Converts a given value into a boolean.
  value_obj to_boolean(const value_obj& value);

  // Converts a given value into a string.
  value_obj to_string(const value_obj& value);

  // ===========================================================================================
  // global_obj manipulation

  // Returns the global that corresponds to a given hashed identifier.
  const value_obj& get_global(uint32_t hash);

  // Sets the global that corresponds to a given hashed identifier to a given value.
  void set_global(uint32_t hash, const value_obj& value);

  // ===========================================================================================
  // Function manipulation

  // Standard return. Returns from the current function with an optional value.
  void native_return(const value_obj& return_value);

  // Calls the given tfunction object with a given argument count.
  void native_call(tfunction* target, size_t argc);

  // Calls the method that lives in a given index of a given object with a given argument count.
  void method_call(object_obj* object, size_t index, size_t argc);

  // Attempts to call the given value object with the given argument count.
  void call(const value_obj& callee, size_t argc);

  // Returns the current stack frame.
  tfunction* get_stack_frame();

  // Attempts to return the upvalue that lives in the given index of the given closure.
  const value_obj& get_upvalue(tfunction* closure, size_t index);

  // Attempts to set the upvalue that lives in the given index of the given closure to a given
  // value.
  void set_upvalue(tfunction* closure, size_t index, const value_obj& value);

  // Returns the upvalue count of the given closure.
  size_t get_upvalue_count(tfunction* closure);

  // Returns the local count of the given closure.
  size_t get_local_count_closure(tfunction* closure);

  // ===========================================================================================
  // General operations
};

std::string to_string(State*);

} // namespace via

#endif
