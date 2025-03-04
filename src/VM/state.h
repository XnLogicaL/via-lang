// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "instruction.h"
#include "signal.h"

#define VIA_VM_STACK_SIZE 8 * 1024 * 1024 // 8 MBs
#define VIA_REGISTER_COUNT 2048 * 8

namespace via {

// Forward declarations
struct TFunction;
struct TValue;
struct TString;
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
    TFunction *frame = nullptr;
    std::string message = "";
};

// Global state, should only be instantiated once, and shared across all
// State's. (threads)
struct GState {
    std::unordered_map<U32, TString *> stable; // String interning table
    std::unordered_map<U32, TValue> gtable;    // Global environment
    std::atomic<U32> threads{0};               // Thread count

    std::shared_mutex stable_mutex;
    std::mutex gtable_mutex;
    std::mutex symtable_mutex;
};

// More likely to be cached (hopefully...)
struct alignas(64) State {
    // Thread and global state
    U32 id;    // Thread ID
    GState *G; // Global state

    // Instruction pointers
    Instruction *ip = nullptr;  // Current instruction pointer
    Instruction *ihp = nullptr; // Instruction list head pointer
    Instruction *ibp = nullptr; // Instruction list base pointer

    // VM execution state
    GarbageCollector *gc; // Pointer to VM garbage collector state

    // Stack state
    TValue *sbp;    // Stack base pointer
    size_t sp = 0;  // Stack pointer
    size_t ssp = 0; // Saved stack pointer

    // Registers
    TValue *registers;

    // Call and frame management
    TFunction *frame = nullptr;         // Call stack pointer
    size_t argc = 0;                    // Argument count (for CALL and FASTCALLX)
    CallType calltype = CallType::CALL; // Current calling convention

    // VM control and debugging
    bool abort = false;
    ErrorState *err;

    // Thread state
    ThreadState tstate = ThreadState::PAUSED; // Current thread state
    State *sstate = nullptr;                  // Saved thread state

    // Signals
    utils::Signal<> sig_exit;
    utils::Signal<> sig_abort;
    utils::Signal<> sig_error;
    utils::Signal<> sig_fatal;

    ProgramData *program;

    State(GState *, ProgramData *);
    ~State();

    void load(BytecodeHolder &);
};

std::string to_string(State *);

} // namespace via