/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "libutils.h"
#include "types.h"
#include "state.h"

namespace via::lib
{

void base_print(State *);
// Basically `base_print` but ends the string with a line break
void base_println(State *);
void base_error(State *);
void base_exit(State *);
void base_type(State *);
void base_typeof(State *);
void base_tostring(State *);
void base_tonumber(State *);
void base_tobool(State *);
void base_assert(State *);
void base_getmetatable(State *);
void base_setmetatable(State *);
void base_pcall(State *);

void loadbaselib(State *);

} // namespace via::lib
