// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "api.h"

namespace via::lib {

// Returns the "name" aka identifier of the given function
void function_name(State *);
void function_parameters(State *);
void function_returntype(State *);
void function_line(State *);

} // namespace via::lib