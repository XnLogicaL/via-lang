/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "api.h"
#include "state.h"
#include "types.h"
#include "libutils.h"
#include <cmath>

namespace via::lib {

LIB_DECL_FUNCTION(math_exp);
LIB_DECL_FUNCTION(math_log);
LIB_DECL_FUNCTION(math_log10);
LIB_DECL_FUNCTION(math_pow);
LIB_DECL_FUNCTION(math_cos);
LIB_DECL_FUNCTION(math_tan);
LIB_DECL_FUNCTION(math_asin);
LIB_DECL_FUNCTION(math_acos);
LIB_DECL_FUNCTION(math_atan);
LIB_DECL_FUNCTION(math_atan2);
LIB_DECL_FUNCTION(math_sinh);
LIB_DECL_FUNCTION(math_cosh);
LIB_DECL_FUNCTION(math_tanh);
LIB_DECL_FUNCTION(math_abs);
LIB_DECL_FUNCTION(math_min);
LIB_DECL_FUNCTION(math_max);
LIB_DECL_FUNCTION(math_round);
LIB_DECL_FUNCTION(math_floor);
LIB_DECL_FUNCTION(math_ceil);
LIB_DECL_FUNCTION(open_mathlib);

} // namespace via::lib
