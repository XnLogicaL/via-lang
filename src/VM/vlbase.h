/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "libutils.h"
#include "shared.h"
#include "types.h"
#include "state.h"

namespace via::lib
{

void base_print(RTState *);
// Basically `base_print` but ends the string with a line break
void base_println(RTState *);
void base_error(RTState *);
void base_exit(RTState *);
void base_type(RTState *);
void base_typeof(RTState *);
void base_tostring(RTState *);
void base_tonumber(RTState *);
void base_tobool(RTState *);
void base_assert(RTState *);
void base_getmetatable(RTState *);
void base_setmetatable(RTState *);
void base_pcall(RTState *);

void loadbaselib(RTState *);

} // namespace via::lib
