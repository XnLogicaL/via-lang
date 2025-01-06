/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "register.h"

namespace via
{

RAState *rnewstate(RTState *)
{
    auto *state = new RAState;

    void *alloc = std::malloc(sizeof(TValue) * VIA_REGISTER_COUNT);
    state->head = reinterpret_cast<TValue *>(alloc);

    for (GPRegister i = 0; i < VIA_REGISTER_COUNT; i++)
    {
        TValue val;
        val.type = ValueType::Monostate;
        val.next = nullptr;
        val.prev = nullptr;
        rsetregister(state, i, val);
    }

    return state;
}

void rcleanupstate(RAState *R)
{
    std::free(R->head);
    delete R;
}

} // namespace via
