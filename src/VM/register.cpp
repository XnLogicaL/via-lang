/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "register.h"

namespace via
{

RAState::RAState()
{
    void *alloc = std::malloc(sizeof(TValue) * VIA_REGISTER_COUNT);
    this->head = reinterpret_cast<TValue *>(alloc);

    for (RegId i = 0; i < VIA_REGISTER_COUNT; i++)
    {
        TValue val;
        val.type = ValueType::Monostate;
        rsetregister(this, i, val);
    }
}

RAState::~RAState()
{
    std::free(head);
}

} // namespace via
