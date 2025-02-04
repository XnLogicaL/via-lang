/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "api.h"

namespace via::lib
{

// Returns the "name" aka identifier of the given function
void function_name(State *);
void function_parameters(State *);
void function_returntype(State *);
void function_line(State *);

} // namespace via::lib