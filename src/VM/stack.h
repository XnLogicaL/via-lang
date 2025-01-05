/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "bitutils.h"
#include "common.h"
#include "core.h"
#include "instruction.h"
#include "types.h"
#include <cstdlib>
#include <stdexcept>

// Stack depth limit
#ifndef VIA_STACK_SIZE
    #define VIA_STACK_SIZE 512
#endif

/* Stack diagram

Before call:
|-------|
|0      | FP        <- Root frame pointer
|1      | MAIN*     <- Root function object pointer
|2      | ARG_N*
...
|N+2    | ARG_1*
|-------|

During call:
|-------|
|0      | FP        <- Root frame pointer
|1      | MAIN*     <- Root function object pointer
|2      | ARG_N*    <- Last argument first
...
|N+2    | ARG_1*    <- First argument
|N+3    | FP        <- [Frame pointer] Frame pointer
|N+4    | CALLEE*   <- Pointer to callee function object, contains metadata
|N+5    | LOCAL_X*  <- [Stack pointer] Local variables
...
|N+X+5  | LOCAL_1*
|N+X+6  | RET_M*    <- Return values at the bottom (masked)
...
|N+X+M+7| RET_1*
|-------|

After call:
|-------|
|0      | FP        <- Root frame pointer
|1      | MAIN*     <- Root function object pointer
|2      | RET_M*
...
|M+2    | RET_1*
|-------|

We're using a bitmasking system to tag pointers in a way that makes them distinguishable from each other.
For example, to get to the last frame pointer, you would pop until you hit a pointer masked with 0x4.

*/

namespace via
{

struct TStack
{
    uintptr_t *sbp; // Bottom of the stack
    size_t sp = 0;
    size_t fp = 0;
    TFunction *pc = nullptr;
};

// Push a value onto the stack
VIA_FORCEINLINE void tspush(TStack *VIA_RESTRICT S, uintptr_t ptr) noexcept
{
    S->sbp[S->sp++] = ptr;
}

VIA_FORCEINLINE void tspusharg(TStack *VIA_RESTRICT S, TValue *val) noexcept
{
    uintptr_t ptr = TAG_ARGUMENT(val);
    tspush(S, ptr);
}

VIA_FORCEINLINE void tspushret(TStack *VIA_RESTRICT S, TValue *val) noexcept
{
    uintptr_t ptr = TAG_RETURN_VALUE(val);
    tspush(S, ptr);
}

// Pop a value from the stack
VIA_FORCEINLINE uintptr_t tspop(TStack *VIA_RESTRICT S) noexcept
{
    S->sp--;
    uintptr_t value = S->sbp[S->sp];
    return value & ~0x7;
}

// Get the top value from the stack
VIA_FORCEINLINE uintptr_t tstop(TStack *VIA_RESTRICT S) noexcept
{
    return S->sbp[S->sp];
}

// Reset the stack pointer to the base
VIA_FORCEINLINE void tsflush(TStack *VIA_RESTRICT S) noexcept
{
    S->sp = 0;
}

VIA_FORCEINLINE void tscall(TStack *VIA_RESTRICT S, TFunction *callee)
{
    tspush(S, TAG_STACK_FRAME(S->pc));
    tspush(S, TAG_STACK_FRAME(S->fp));
    S->fp = S->sp;
    S->pc = callee;
}

VIA_FORCEINLINE void tsret(TStack *VIA_RESTRICT S)
{
    S->fp = reinterpret_cast<uintptr_t>(tspop(S));   // Restore frame pointer
    S->pc = reinterpret_cast<TFunction *>(tspop(S)); // Restore return address (FunctionObject*)
    S->sp = S->fp;                                   // Reset stack pointer
}

// Create a new stack state
VIA_FORCEINLINE TStack *tsnewstate()
{
    // Allocate memory for the stack state
    TStack *state = new TStack;
    // Allocate memory for the stack
    void *mem = std::malloc(VIA_STACK_SIZE * sizeof(uintptr_t));
    state->sbp = static_cast<uintptr_t *>(mem);

    if (!state->sbp)
    {
        delete state;
        throw std::bad_alloc();
    }

    // Initialize the stack pointer
    state->sp = 0;
    return state;
}

// Delete a stack state
VIA_FORCEINLINE void tscleanupstate(TStack *S)
{
    std::free(S->sbp); // Free the stack memory
    delete S;          // Free the stack state
}

} // namespace via
