/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "gc.h"
#include "shared.h"

namespace via
{

class Global;
template<typename T>
class Stack;
class StackFrame;
class GarbageCollector;
class RegisterAllocator;
struct viaString;

static uint32_t __thread_id = 0;

using VMStack = Stack<StackFrame>;
using Labels = viaHashMap<viaLabelKey, viaInstruction *>;
using STable = viaHashMap<viaHash, viaString *>;

enum class viaThreadState
{
    RUNNING,
    PAUSED,
    DEAD
};

struct viaGlobalState
{
    Global *global; // Pointer to VM Global environment
    STable *stable; // Global string lookup table, derrived from Lua's string interning
};

// More likely to be cached (hopefully...)
struct alignas(64) viaState
{
    uint32_t id; // Thread id

    viaGlobalState *G; // Global state

    viaInstruction *ip;  // Instruction pointer
    viaInstruction *ihp; // Instruction list head
    viaInstruction *ibp; // Instruction list base

    VMStack *stack;            // Pointer to VM Stack
    Labels *labels;            // Pointer to VM Label address table (LAT)
    RegisterAllocator *ralloc; // VM viaRegister allocator
    viaGCState *gc;            // Pointer to VM Garbage collector state

    int exitc;         // VM exit code
    const char *exitm; // VM exit message

    bool abrt;         // Aborts on the next VM cycle
    bool skip;         // Skips the next instruction on the next VM cycle
    bool yield;        // Tells the VM to yield or not on the next VM cycle (debounces, meaning resets every VM cycle)
    bool restorestate; // Tells the VM to restore the state on the next VM cycle (to sstate)
    bool hasself;      // Tells if there is a self argument loaded into AR0 (resets every call)

    float yieldfor; // Time (in ms) to yield on the next VM cycle (only goes thru if V->yield is true)
    int argc;       // Argument count for the currently being-called function

    viaThreadState ts; // Thread state
    viaState *sstate;  // Saved state
};

} // namespace via