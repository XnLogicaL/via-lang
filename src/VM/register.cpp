/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "register.h"

namespace via
{

viaRAllocatorState *viaR_newstate(viaState *V)
{
    auto *state = new viaRAllocatorState;

    void *alloc = std::malloc(sizeof(viaValue) * VIA_REGISTERCOUNT);
    state->head = reinterpret_cast<viaValue *>(alloc);

    // Initialize registers with Nil
    for (viaRegister i = 0; i < VIA_REGISTERCOUNT; i++)
        viaR_setregister(state, i, viaT_stackvalue(V));

    return state;
}

} // namespace via
