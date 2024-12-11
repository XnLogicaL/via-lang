/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "register.h"

namespace via
{

void viaR_initialize(viaRAllocatorState *R)
{
    for (viaRegister i = 0; i < VIA_REGISTERCOUNT; i++)
    {
        viaValue monostate_val;
        monostate_val.type = viaValueType::Monostate;
        monostate_val.next = nullptr;
        monostate_val.prev = nullptr;

        viaR_setregister(R, i, monostate_val);
    }
}

viaRAllocatorState *viaR_newstate(viaState *)
{
    auto *state = new viaRAllocatorState;

    void *alloc = std::malloc(sizeof(viaValue) * VIA_REGISTERCOUNT);
    state->head = reinterpret_cast<viaValue *>(alloc);

    viaR_initialize(state);

    return state;
}

} // namespace via
