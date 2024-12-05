/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "bytecode.h"
#include "shared.h"
#include "state.h"
#include "api.h"
#include "types.h"
#include "libutils.h"

#include <cmath>
#include <chrono>

namespace via::lib
{

// Function to generate a random double in the range [a, b]
viaNumber pcg32_range(viaNumber a, viaNumber b);
void rand_range(viaState *V);
void rand_int(viaState *V);
void viaL_loadrandlib(viaState *V);

} // namespace via::lib