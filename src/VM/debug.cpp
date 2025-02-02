/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "debug.h"

namespace via
{

void dbgprintregistermap(RTState *VIA_RESTRICT V = nullptr, size_t count = VIA_REGISTER_COUNT)
{
    for (RegId reg = 0; reg <= count; reg++)
    {
        TValue *val = rgetregister(V->ralloc, reg);
        TValue &val_clone = *val;
        tostring(V, val_clone);
        std::string fmt = std::format("R{}<'{}':'{}'@{}>", reg, ENUM_NAME(val->type), val_clone.val_string->ptr, reinterpret_cast<const void *>(val));
        std::cout << fmt << "\n";
    }
}

void dbgprintstack(RTState *VIA_RESTRICT V = nullptr)
{
    StkVal *sp = V->stack->sbp + V->stack->sp;
    for (StkVal *i = sp; i < V->stack->sbp; i++)
        std::cout << std::format("|{}| {}\n", reinterpret_cast<const void *>(i), reinterpret_cast<const void *>(i));
}

} // namespace via