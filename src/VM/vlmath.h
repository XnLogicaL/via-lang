/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "state.h"
#include "types.h"
#include "libutils.h"
#include <cmath>

namespace via::lib
{

void math_exp(RTState *V);
void math_log(RTState *V);
void math_log10(RTState *V);
void math_pow(RTState *V);
void math_cos(RTState *V);
void math_tan(RTState *V);
void math_asin(RTState *V);
void math_acos(RTState *V);
void math_atan(RTState *V);
void math_atan2(RTState *V);
void math_sinh(RTState *V);
void math_cosh(RTState *V);
void math_tanh(RTState *V);
void math_abs(RTState *V);
void math_min(RTState *V);
void math_max(RTState *V);
void math_round(RTState *V);
void math_floor(RTState *V);
void math_ceil(RTState *V);
void loadmathlib(RTState *V);

} // namespace via::lib
