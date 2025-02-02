/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "instruction.h"
#include "types.h"

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

    TStack();
    ~TStack();
};

// Push a value onto the stack
VIA_MAXOPTIMIZE void tspush(TStack *VIA_RESTRICT S, StkVal &val) noexcept
{
    S->sbp[S->sp++] = val.clone();
}

// Pop a value from the stack
VIA_MAXOPTIMIZE StkVal tspop(TStack *VIA_RESTRICT S) noexcept
{
    StkVal &val = S->sbp[S->sp--]; // Copy the stack top value in order to avoid possible memory bugs
    return val.clone();
}

// Get the top value from the stack
VIA_MAXOPTIMIZE StkVal tstop(TStack *VIA_RESTRICT S) noexcept
{
    TValue &val = S->sbp[S->sp];
    return val.clone();
}

// Reset the stack pointer to the base
VIA_MAXOPTIMIZE void tsflush(TStack *VIA_RESTRICT S) noexcept
{
    S->sp = 0;
}

} // namespace via
