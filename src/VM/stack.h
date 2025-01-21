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

void tspush(TStack *VIA_RESTRICT S, StkVal val) noexcept;
StkVal tspop(TStack *VIA_RESTRICT S) noexcept;
StkVal tstop(TStack *VIA_RESTRICT S) noexcept;
void tsflush(TStack *VIA_RESTRICT S) noexcept;

} // namespace via
