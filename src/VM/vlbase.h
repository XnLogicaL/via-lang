/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "libutils.h"
#include "shared.h"
#include "types.h"
#include "state.h"

namespace via::lib
{

void base_print(viaState *V);
// Basically `base_print` but ends the string with a line break
void base_println(viaState *V);
void base_error(viaState *V);
void base_exit(viaState *V);
void base_type(viaState *V);
void base_typeof(viaState *V);
void base_tostring(viaState *V);
void base_tonumber(viaState *V);
void base_tobool(viaState *V);
void base_assert(viaState *V);
void base_getmetatable(viaState *V);
void base_setmetatable(viaState *V);

void viaL_loadbaselib(viaState *V);

} // namespace via::lib
