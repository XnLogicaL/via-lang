/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "libutils.h"
#include "shared.h"
#include "types.h"
#include "state.h"

namespace via::lib
{

void base_print(viaState *);
// Basically `base_print` but ends the string with a line break
void base_println(viaState *);
void base_error(viaState *);
void base_exit(viaState *);
void base_type(viaState *);
void base_typeof(viaState *);
void base_tostring(viaState *);
void base_tonumber(viaState *);
void base_tobool(viaState *);
void base_assert(viaState *);
void base_getmetatable(viaState *);
void base_setmetatable(viaState *);
void base_pcall(viaState *);

void viaL_loadbaselib(viaState *);

} // namespace via::lib
