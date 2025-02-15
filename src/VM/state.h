/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"
#include "signal.h"

// Identifier of the defacto "main" function
// Kinda useless but it can stay
#ifndef VIA_MAIN_ID
    #define VIA_MAIN_ID ("__main")
#endif

#ifndef VIA_VM_STACK_SIZE
    #define VIA_VM_STACK_SIZE (8 * 1024 * 1024) // 8 MBs
#endif

namespace via {

// Forward declarations
struct TFunction;
struct TValue;
struct TStack;
struct GarbageCollector;
struct RAState;
struct TString;

// Type aliases for convenience, not much else
using StrTable = std::unordered_map<Hash, TString *>;
using GlbTable = std::unordered_map<kGlobId, TValue>;
using kTable = std::vector<TValue>;
using SymTable = std::vector<std::string>;

// Calling convention
enum class CallType {
    NOCALL,
    CALL,
    FASTCALL,
};

// State of an State (thread) execution
enum class ThreadState { RUNNING, PAUSED, DEAD };

struct ErrorState {
    TFunction *frame = nullptr;
    std::string message = "";
};

// Global state, should only be instantiated once, and shared across all State's. (threads)
struct GState {
    StrTable stable;   // Global string lookup table, derrived from Lua's string interning
    GlbTable gtable;   // Global environment
    kTable ktable;     // Constant table. Provided by the compiler.
    SymTable symtable; // Symbol table, maps the stack offsets of variables to their identifiers. Provided by the compiler.
    ThreadId threads;  // Number of threads

    GState();
    ~GState() = default;
};

// More likely to be cached (hopefully...)
struct alignas(64) State {
    // Thread and global state
    ThreadId id; // Thread ID
    GState *G;   // Global state

    // Instruction pointers
    Instruction *ip = nullptr;  // Current instruction pointer
    Instruction *ihp = nullptr; // Instruction list head pointer
    Instruction *ibp = nullptr; // Instruction list base pointer

    // VM execution state
    RAState *ralloc;      // Pointer to VM register allocator state
    GarbageCollector *gc; // Pointer to VM garbage collector state

    // Stack state
    TValue *sbp;    // Stack base pointer
    size_t sp = 0;  // Stack pointer
    size_t ssp = 0; // Saved stack pointer

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

    State(GState *, ProgramData &);
    ~State();

    void load(BytecodeHolder &);
};

} // namespace via