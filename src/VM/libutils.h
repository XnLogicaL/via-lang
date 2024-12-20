/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "shared.h"
#include "types.h"
#include "state.h"
#include "instruction.h"

#define LIB_ASSERT(cond, msg) \
    via_assert(V, (cond), (msg)); \
    if (!(cond)) \
        return;

#define WRAPVAL(val) viaT_stackvalue(V, new viaCFunction{.ptr = val, .error_handler = false})
#define ARG_MISMATCH(i, e, g) std::format("Expected {}, got {} for argument #{}\n", (e), (g), (i))

namespace via::lib
{

viaValue *viaL_quickindex(viaState *, viaTable *, const char *);
void viaL_pusharguments(viaState *, std::vector<viaValue>);

} // namespace via::lib