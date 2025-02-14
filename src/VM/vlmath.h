/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "state.h"
#include "types.h"
#include "libutils.h"
#include <cmath>

namespace via::lib {

void math_exp(State *V);
void math_log(State *V);
void math_log10(State *V);
void math_pow(State *V);
void math_cos(State *V);
void math_tan(State *V);
void math_asin(State *V);
void math_acos(State *V);
void math_atan(State *V);
void math_atan2(State *V);
void math_sinh(State *V);
void math_cosh(State *V);
void math_tanh(State *V);
void math_abs(State *V);
void math_min(State *V);
void math_max(State *V);
void math_round(State *V);
void math_floor(State *V);
void math_ceil(State *V);
void loadmathlib(State *V);

} // namespace via::lib
