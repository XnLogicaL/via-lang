/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"

// Identifier of the defacto "main" function
// Kinda useless but it can stay
#ifndef VIA_MAIN_ID
    #define VIA_MAIN_ID ("__main")
#endif

#ifndef VIA_VM_STACK_SIZE
    #define VIA_VM_STACK_SIZE (8 * 1024 * 1024) // 8 MBs
#endif

namespace via
{

// Forward declarations
struct TFunction;
struct TValue;
struct TStack;
struct GCState;
struct RAState;
struct TString;

// Type aliases for convenience, not much else
using StrTable = std::unordered_map<Hash, TString *>;
using GlbTable = std::unordered_map<kGlobId, TValue>;
using kTable = std::vector<TValue>;
using SymTable = std::vector<std::string>;

// Calling convention
enum class CallType
{
    NOCALL,
    CALL,
    FASTCALL,
};

// State of an State (thread) execution
enum class ThreadState
{
    RUNNING,
    PAUSED,
    DEAD
};

// Global state, should only be instantiated once, and shared across all State's. (threads)
struct GState
{
    StrTable *stable;   // Global string lookup table, derrived from Lua's string interning
    GlbTable *gtable;   // Global environment
    kTable *ktable;     // Constant table. Provided by the compiler.
    SymTable *symtable; // Symbol table, maps the stack offsets of variables to their identifiers. Provided by the compiler.
    ThreadId threads;   // Number of threads

    GState();
    ~GState();
};

// More likely to be cached (hopefully...)
struct alignas(64) State
{
    // Thread and global state
    ThreadId id; // Thread ID
    GState *G;   // Global state

    // Instruction pointers
    Instruction *ip;  // Current instruction pointer
    Instruction *ihp; // Instruction list head pointer
    Instruction *ibp; // Instruction list base pointer

    // VM execution state
    RAState *ralloc; // Pointer to VM register allocator state
    GCState *gc;     // Pointer to VM garbage collector state

    // Stack state
    TValue *sbp; // Stack base pointer
    size_t sp;   // Stack pointer
    size_t ssp;  // Saved stack pointer

    // Call and frame management
    TFunction *frame;  // Call stack pointer
    size_t argc;       // Argument count (for CALL and FASTCALLX)
    CallType calltype; // Current calling convention

    // VM control and debugging
    size_t exitc;      // VM exit code
    const char *exitm; // VM exit message
    bool abrt;         // Abort on the next VM cycle
    bool skip;         // Skip the next instruction on the next VM cycle
    bool yield;        // Yield on the next VM cycle (debounced)
    bool restorestate; // Restore state on the next VM cycle (to `sstate`)
    YldTime yieldfor;  // Time (in ms) to yield on the next VM cycle (if `yield` is true)

    // Thread state
    ThreadState tstate; // Current thread state
    State *sstate;      // Saved thread state

    State(GState *, ProgramData &);
    ~State();

    void load(BytecodeHolder &);
};

} // namespace via