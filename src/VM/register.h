/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "types.h"

#define VIA_REGISTER_COUNT uintptr_t(128)

namespace via
{

struct State;
struct RAState
{
    TValue *head;

    RAState();
    ~RAState();
};

VIA_MAXOPTIMIZE TValue *rgetregister(RAState *R, RegId reg)
{
    TValue *ptr = R->head + reg;
    return ptr;
}

VIA_MAXOPTIMIZE void rsetregister(RAState *R, RegId reg, TValue &val)
{
    TValue *addr = R->head + reg;
    *addr = std::move(val);
}

} // namespace via