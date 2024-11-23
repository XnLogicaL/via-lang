/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "global.h"

namespace via
{

viaExitCode Global::set_global(viaState *, viaGlobalIdentifier k, viaValue v)
{
    auto it = consts.find(k);

    if (it != consts.end())
        // Return 1 (error) if global already exists
        return 1;

    consts.emplace(k, v);
    return 0;
}

viaValue *Global::get_global(viaState *V, viaGlobalIdentifier k)
{
    auto it = consts.find(k);

    if (it == consts.end())
        // Return nil if global not found
        return viaT_newvalue(V);

    return &it->second;
}

} // namespace via