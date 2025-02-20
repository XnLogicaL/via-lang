// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "api.h"
#include "state.h"

namespace via {

void debug_traceback(State *);
void loaddebuglib(State *);

} // namespace via