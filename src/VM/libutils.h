/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "types.h"
#include "state.h"
#include "instruction.h"

#define LIB_ASSERT(cond, msg) \
    if (!(cond)) \
    { \
        setexitdata(V, 1, msg); \
        return; \
    }

#define WRAPVAL(val) stackvalue(V, new TCFunction{.ptr = val, .error_handler = false})
#define ARG_MISMATCH(i, e, g) std::format("Expected {}, got {} for argument #{}\n", (e), (g), (i))

namespace via::lib
{

TValue *quickindex(RTState *, TTable *, const char *);
void pusharguments(RTState *, std::vector<TValue>);

} // namespace via::lib