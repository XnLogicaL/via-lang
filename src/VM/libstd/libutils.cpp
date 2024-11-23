/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "libutils.h"

namespace via::lib
{

viaRegister viaL_getargument(viaState *, viaRegisterOffset off)
{
    return {viaRegisterType::AR, off};
}

viaRegister viaL_getreturn(viaState *, viaRegisterOffset off)
{
    return {viaRegisterType::RR, off};
}

void viaL_loadargs(viaState *V, std::array<viaValue, 16> args)
{
    size_t i = 0;
    for (viaValue val : args)
    {
        viaRegister R = viaL_getargument(V, i++);
        via_setregister(V, R, val);
    }
}

viaValue *viaL_quickindex(viaState *V, viaTable *T, viaRawString K)
{
    return via_gettableindex(V, T, viaT_hashstring(V, K), false);
}

} // namespace via::lib