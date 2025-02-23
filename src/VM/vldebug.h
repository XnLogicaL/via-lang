// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GPL v3.           |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "api.h"
#include "state.h"

namespace via {

void debug_traceback(State *);
void loaddebuglib(State *);

} // namespace via