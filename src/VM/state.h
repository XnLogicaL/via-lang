/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"

// Identifier of the defacto "main" function
// Kinda useless but it can stay
#ifndef VIA_MAIN_ID
    #define VIA_MAIN_ID ("__main__")
#endif

/*
 * Context switching in via consists of 2 steps;
 * - Saving the state
 * - Restoring the state (either by setting V->savestate to true
 * (requires waiting for the next VM cycle, exists for multithreading support) or by calling restorestate(V))
 * This allows for a very fast context switching systen--at least one with minimal overhead
 * Context switching in via has some special traits; for example,
 * the state (e.g stack state) of pointers inside the RTState
 * object are never saved, instead they are passed onto the copy as-is, since they're pointers,
 * creating an efficient way to "shallow-save" the state of the VM
 */
namespace via
{

/*
 * Stores the amount of threads
 * Has this attribute because
 * the compiler doesn't like the fact that it's used in other translation units but not this particular one
 */
#if defined(__GNUC__) || defined(__clang__)
[[maybe_unused]]
#endif
static ThreadId __thread_id__ = 0;

// Forward declarations
struct TFunction;
struct TValue;

struct TStack;
struct TStackFrame;
struct GCState;
struct RAState;
struct TString;

// Type aliases for convenience, not much else
using LblMap = HashMap<LabelId, Instruction *>;
using STable = HashMap<Hash, TString *>;

// Calling convention that is actively being used by the VM
enum class CallType : uint8_t
{
    NOCALL,
    CALL,
    FASTCALL,
};

enum class ThreadState : uint8_t
{
    RUNNING,
    PAUSED,
    DEAD
};

struct GState
{
    STable *stable; // Global string lookup table, derrived from Lua's string interning
};

// More likely to be cached (hopefully...)
struct alignas(64) RTState
{
    ThreadId id;        // Thread id
    GState *G;          // Global state
    Instruction *ip;    // Instruction pointer
    Instruction *ihp;   // Instruction list head
    Instruction *ibp;   // Instruction list base
    TStack *stack;      // Pointer to VM Stack
    RAState *ralloc;    // Pointer to VM Register allocator state
    LblMap *labels;     // Pointer to VM Label address table (LAT)
    GCState *gc;        // Pointer to VM Garbage collector state
    uintptr_t ssp;      // Saved stack pointer
    TFunction *frame;   // Callstack pointer
    CallArgc argc;      // Argument count, for both CALL and FASTCALLX
    CallType calltype;  // Stores the current calling convention
    TValue *heapvhead;  // Pointer to the first heap allocated value
    int exitc;          // VM exit code
    const char *exitm;  // VM exit message
    bool abrt;          // Aborts on the next VM cycle
    bool skip;          // Skips the next instruction on the next VM cycle
    bool yield;         // Tells the VM to yield or not on the next VM cycle (debounces, meaning gets flipped every VM clock, if set to true)
    bool restorestate;  // Tells the VM to restore the state on the next VM cycle (to sstate)
    float yieldfor;     // Time (in ms) to yield on the next VM cycle (only goes thru if V->yield is true)
    ThreadState tstate; // Thread state
    RTState *sstate;    // Saved state
};

// Creates a new global state object
GState *stnewgstate();
// Creats a new state object
RTState *stnewstate(const std::vector<Instruction> &);
// Cleans up a global state object
void stcleanupgstate(GState *);
// Cleans up a state object
void stcleanupstate(RTState *);
// Returns the thread count, no matter their state
// Equivalent to `__thread_id__`
ThreadId stthreadcount(RTState *);

} // namespace via