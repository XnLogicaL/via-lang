/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "libutils.h"

namespace via::lib
{

void pusharguments(RTState *V, std::vector<TValue> args)
{
    for (TValue val : args)
        pusharg(V, val);
}

TValue *quickindex(RTState *V, TTable *T, const char *K)
{
    return gettableindex(V, T, hashstring(V, K), false);
}

} // namespace via::lib