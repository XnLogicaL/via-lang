/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "shared.h"
#include "types.h"

#define VIA_REGISTERCOUNT uintptr_t(2048)

namespace via
{

struct viaState;
struct viaRAllocatorState
{
    viaValue *head;
};

inline viaValue *viaR_getregister(viaRAllocatorState *R, viaRegister reg)
{
    viaValue *ptr = R->head + reg;
    return ptr;
}

inline void viaR_setregister(viaRAllocatorState *R, viaRegister reg, viaValue val)
{
    viaValue *addr = R->head + reg;
    *addr = val;
}

viaRAllocatorState *viaR_newstate(viaState *);
void viaR_initialize(viaRAllocatorState *);

} // namespace via