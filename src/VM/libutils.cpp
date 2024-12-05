/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "libutils.h"

namespace via::lib
{

void viaL_pusharguments(viaState *V, std::vector<viaValue> args)
{
    for (viaValue val : args)
        via_pushargument(V, val);
}

viaValue *viaL_quickindex(viaState *V, viaTable *T, viaRawString_t K)
{
    return via_gettableindex(V, T, viaT_hashstring(V, K), false);
}

} // namespace via::lib