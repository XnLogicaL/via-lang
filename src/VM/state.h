/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "bytecode.h"
#include "core.h"
#include "gc.h"
#include "global.h"
#include "register.h"
#include "stack.h"
#include <cstdint>

namespace via
{

namespace VM
{

static uint32_t __thread_id = 0;

enum class viaThreadState
{
    RUNNING,
    PAUSED,
    DEAD
};

struct viaGlobalState
{
    Global *global; // Pointer to VM Global environment
};

// More likely to be cached (hopefully...)
struct alignas(64) viaState
{
    using VMStack = Stack<StackFrame>;
    using Labels = std::unordered_map<std::string_view, viaInstruction *>;

    uint32_t id; // Thread id

    viaGlobalState *G; // Global state

    viaInstruction *ip;  // Instruction pointer
    viaInstruction *ihp; // Instruction list head
    viaInstruction *ibp; // Instruction list base

    VMStack *stack;           // Pointer to VM Stack
    Labels *labels;           // Pointer to VM Label address table (LAT)
    GarbageCollector *gc;     // Pointer to VM Garbage collector
    RegisterAllocator ralloc; // VM viaRegister allocator

    int exitc;         // VM exit code
    const char *exitm; // VM exit message

    bool abrt;         // Aborts on the next VM cycle
    bool skip;         // Skips the next instruction on the next VM cycle
    bool yield;        // Tells the VM to yield or not on the next VM cycle (debounces, meaning resets every VM cycle)
    bool restorestate; // Tells the VM to restore the state on the next VM cycle (to sstate)

    float yieldfor; // Time (in ms) to yield on the next VM cycle (only goes thru if V->yield is true)

    viaThreadState ts; // Thread state
    viaState *sstate;  // Saved state
};

} // namespace VM

using viaState = VM::viaState;

} // namespace via