/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "register.h"

namespace via
{

void rcleanupstate(RAState *R)
{
    std::free(R->head);
    delete R;
}

void rinitialize(RAState *R)
{
    for (GPRegister i = 0; i < VIA_REGISTER_COUNT; i++)
    {
        TValue monostate_val;
        monostate_val.type = ValueType::Monostate;
        monostate_val.next = nullptr;
        monostate_val.prev = nullptr;

        rsetregister(R, i, monostate_val);
    }
}

RAState *rnewstate(RTState *)
{
    auto *state = new RAState;

    void *alloc = std::malloc(sizeof(TValue) * VIA_REGISTER_COUNT);
    state->head = reinterpret_cast<TValue *>(alloc);

    rinitialize(state);

    return state;
}

} // namespace via
