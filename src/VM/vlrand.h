/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "bytecode.h"
#include "state.h"
#include "api.h"
#include "types.h"
#include "libutils.h"
#include <cmath>
#include <chrono>

namespace via::lib {

// Function to generate a random double in the range [a, b].
TNumber pcg32_range(TNumber a, TNumber b);
void rand_range(State *V);
void rand_int(State *V);
void loadrandlib(State *V);

} // namespace via::lib