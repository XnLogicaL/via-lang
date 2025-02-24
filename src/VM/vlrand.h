// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "bytecode.h"
#include "state.h"
#include "api.h"
#include "types.h"
#include "libutils.h"
#include <cmath>
#include <chrono>

namespace via::lib {

void rand_range(State *V);
void rand_int(State *V);
void loadrandlib(State *V);

} // namespace via::lib