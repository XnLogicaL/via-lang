/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "types.h"

namespace via
{

VIA_FORCEINLINE TNumber lerp(TNumber a, TNumber b, TNumber t)
{
    return a + (b - a) * t;
}

} // namespace via