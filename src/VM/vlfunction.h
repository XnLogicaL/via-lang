/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "shared.h"
#include "api.h"

namespace via::lib
{

// Returns the "name" aka identifier of the given function
void function_name(viaState *);
void function_params(viaState *);
void function_returns(viaState *);
void function_line(viaState *);

} // namespace via::lib