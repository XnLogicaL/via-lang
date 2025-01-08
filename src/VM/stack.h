/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"
#include "types.h"
#include <cstdlib>
#include <stdexcept>

// Stack depth limit
#ifndef VIA_STACK_SIZE
    #define VIA_STACK_SIZE 512
#endif

namespace via
{

struct TStack
{
    StkAddr sbp; // Bottom of the stack
    StkPos sp = 0;
};

// Push a value onto the stack
VIA_FORCEINLINE void tspush(TStack *VIA_RESTRICT S, StkVal ptr) noexcept
{
    S->sbp[S->sp++] = ptr;
}

// Push a value onto the stack
VIA_FORCEINLINE void tspush(TStack *VIA_RESTRICT S, TValue *ptr) noexcept
{
    S->sbp[S->sp++] = reinterpret_cast<StkVal>(ptr);
}

// Pop a value from the stack
VIA_FORCEINLINE StkVal tspop(TStack *VIA_RESTRICT S) noexcept
{
    S->sp--;
    StkVal value = S->sbp[S->sp];
    return value & ~0x7;
}

// Get the top value from the stack
VIA_FORCEINLINE StkVal tstop(TStack *VIA_RESTRICT S) noexcept
{
    return S->sbp[S->sp];
}

// Reset the stack pointer to the base
VIA_FORCEINLINE void tsflush(TStack *VIA_RESTRICT S) noexcept
{
    S->sp = 0;
}

// Create a new stack state
VIA_FORCEINLINE TStack *tsnewstate()
{
    // Allocate memory for the stack state
    TStack *state = new TStack;
    // Allocate memory for the stack
    void *mem = std::malloc(VIA_STACK_SIZE * sizeof(StkVal));
    state->sbp = static_cast<StkVal *>(mem);

    VIA_ASSERT_SILENT(state->sbp, "TStack allocation failed");

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
