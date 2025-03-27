// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_STATE_H
#define _VIA_STATE_H

#include "common.h"
#include "instruction.h"
#include "object.h"
#include "signal.h"

#define VIA_VM_STACK_SIZE  2048
#define VIA_REGISTER_COUNT 0xFFFF

VIA_NAMESPACE_BEGIN

// Forward declarations
struct TFunction;
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
    TFunction*  frame   = nullptr;
    std::string message = "";
};

// Global state, should only be instantiated once, and shared across all
// State's. (threads)
struct GState {
    std::unordered_map<uint32_t, TString*> stable;     // String interning table
    std::unordered_map<uint32_t, TValue>   gtable;     // Global environment
    std::atomic<uint32_t>                  threads{0}; // Thread count

    std::shared_mutex stable_mutex;
    std::mutex        gtable_mutex;
    std::mutex        symtable_mutex;
};

struct alignas(64) State {
    // Thread and global state
    uint32_t id; // Thread ID
    GState*  G;  // Global state

    // Instruction pointers
    Instruction* ip   = nullptr; // Current instruction pointer
    Instruction* ibp  = nullptr; // Instruction list begin pointer
    Instruction* iep  = nullptr; // Instruction list end pointer
    Instruction* sibp = nullptr;
    Instruction* siep = nullptr;

    // Stack state
    TValue* sbp;    // Stack base pointer
    size_t  sp = 0; // Stack pointer

    // Registers
    TValue* registers;

    // Labels
    Instruction** labels;

    // Call and frame management
    TFunction* frame = nullptr; // Call stack pointer

    // VM control and debugging
    bool        abort = false;
    ErrorState* err;

    // Thread state
    ThreadState tstate = ThreadState::PAUSED; // Current thread state

    // Signals
    utils::Signal<> sig_exit;
    utils::Signal<> sig_abort;
    utils::Signal<> sig_error;
    utils::Signal<> sig_fatal;

    TransUnitContext& unit_ctx;

    VIA_NON_COPYABLE(State);
    VIA_CUSTOM_DESTRUCTOR(State);

    State(GState* global, TransUnitContext& unit_ctx);

    void load(const BytecodeHolder& bytecode);

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
    TValue& get_register(Operand reg);

    // Sets a given register to a given value.
    void set_register(Operand reg, const TValue& value);

    // ===========================================================================================
    // Comparison and metadata

    // Returns whether if a given value has a heap-allocated component.
    bool is_heap(const TValue& value);

    // Compares two given values.
    bool compare(const TValue& left, const TValue& right);

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
    void push(const TValue& value);

    // Drops a value from the stack, frees the resources of the dropped value.
    void drop();

    // Pops a value from the stack and returns it.
    TValue pop();

    // Returns the top value on the stack.
    const TValue& top();

    // ===========================================================================================
    // Advanced stack manipulation

    // Sets the value at a given position on the stack to a given value.
    void set_stack(size_t position, TValue value);

    // Returns the stack value at a given position.
    const TValue& get_stack(size_t position);

    // Returns the stack value at a given offset relative to the current stack-frame's stack
    // pointer.
    const TValue& get_argument(size_t offset);

    // Returns the size of stack.
    size_t stack_size();

    // ===========================================================================================
    // Value manipulation

    // Attempts to convert a given value into an integer.
    TValue to_integer(const TValue& value);

    // Attempts to convert a given value into a float.
    TValue to_float(const TValue& value);

    // Converts a given value into a boolean.
    TValue to_boolean(const TValue& value);

    // Converts a given value into a string.
    TValue to_string(const TValue& value);

    // ===========================================================================================
    // Global manipulation

    // Returns the global that corresponds to a given hashed identifier.
    const TValue& get_global(uint32_t hash);

    // Sets the global that corresponds to a given hashed identifier to a given value.
    void set_global(uint32_t hash, const TValue& value);

    // ===========================================================================================
    // Function manipulation

    // Standard return. Returns from the current function with an optional value.
    void native_return(const TValue& return_value);

    // Calls the given TFunction object with a given argument count.
    void native_call(TFunction* target, size_t argc);

    // Calls the method that lives in a given index of a given object with a given argument count.
    void method_call(TObject* object, size_t index, size_t argc);

    // Attempts to call the given value object with the given argument count.
    void call(const TValue& callee, size_t argc);

    // Returns the current stack frame.
    TFunction* get_stack_frame();

    // Attempts to return the upvalue that lives in the given index of the given closure.
    const TValue& get_upvalue(TFunction* closure, size_t index);

    // Attempts to set the upvalue that lives in the given index of the given closure to a given
    // value.
    void set_upvalue(TFunction* closure, size_t index, const TValue& value);

    // Returns the upvalue count of the given closure.
    size_t get_upvalue_count(TFunction* closure);

    // Returns the local count of the given closure.
    size_t get_local_count_closure(TFunction* closure);

    // ===========================================================================================
    // General operations
};

std::string to_string(State*);

VIA_NAMESPACE_END

#endif
