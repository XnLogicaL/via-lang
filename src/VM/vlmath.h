/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "shared.h"
#include "state.h"
#include "types.h"
#include "libutils.h"

#include <cmath>

namespace via::lib
{

void math_exp(viaState *V);
void math_log(viaState *V);
void math_log10(viaState *V);
void math_pow(viaState *V);
void math_cos(viaState *V);
void math_tan(viaState *V);
void math_asin(viaState *V);
void math_acos(viaState *V);
void math_atan(viaState *V);
void math_atan2(viaState *V);
void math_sinh(viaState *V);
void math_cosh(viaState *V);
void math_tanh(viaState *V);
void math_abs(viaState *V);
void math_min(viaState *V);
void math_max(viaState *V);
void math_round(viaState *V);
void math_floor(viaState *V);
void math_ceil(viaState *V);
void viaL_loadmathlib(viaState *V);

} // namespace via::lib
