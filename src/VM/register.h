/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "shared.h"
#include "types.h"

#define VIA_REGISTERCOUNT uintptr_t(2048)

namespace via
{

struct RTState;
struct RAState
{
    TValue *head;
};

inline TValue *rgetregister(RAState *R, GPRegister reg)
{
    TValue *ptr = R->head + reg;
    return ptr;
}

inline void rsetregister(RAState *R, GPRegister reg, TValue val)
{
    TValue *addr = R->head + reg;
    *addr = val;
}

RAState *rnewstate(RTState *);
void rcleanupstate(RAState *);
void rinitialize(RAState *);

} // namespace via