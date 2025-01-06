/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "api.h"

namespace via::lib
{

// Returns the "name" aka identifier of the given function
void function_name(RTState *);
void function_parameters(RTState *);
void function_returntype(RTState *);
void function_line(RTState *);

} // namespace via::lib