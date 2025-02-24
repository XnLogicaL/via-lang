// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
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