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

#define WRAPVAL(val) viaT_stackvalue(V, val)

namespace via::lib
{

viaRegister viaL_getargument(viaState *, viaRegisterOffset);
viaRegister viaL_getreturn(viaState *, viaRegisterOffset);
void viaL_loadargs(viaState *, std::array<viaValue, 16>);
viaValue *viaL_quickindex(viaState *, viaTable *, viaRawString);

} // namespace via::lib