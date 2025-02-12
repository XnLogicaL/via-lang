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

    RAState()
    {
        void *alloc = std::malloc(sizeof(TValue) * VIA_REGISTER_COUNT);
        this->head = reinterpret_cast<TValue *>(alloc);

        for (RegId i = 0; i < VIA_REGISTER_COUNT; i++)
        {
            TValue val;
            TValue *addr = head + i;
            val.type = ValueType::monostate;
            *addr = val.clone();
        }
    }

    ~RAState()
    {
        std::free(head);
    }
};

VIA_MAXOPTIMIZE TValue *getregister(State *V, RegId reg)
{
    TValue *ptr = V->ralloc->head + reg;
    return ptr;
}

VIA_MAXOPTIMIZE void setregister(State *V, RegId reg, const TValue &val)
{
    TValue *addr = V->ralloc->head + reg;
    *addr = val.clone();
}

} // namespace via