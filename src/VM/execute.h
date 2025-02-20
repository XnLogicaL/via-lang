// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "state.h"

namespace via {

void vm_save_snapshot(State *VIA_RESTRICT V);
// Main VM function that starts execution
void execute(State *VIA_RESTRICT V);
void kill_thread(State *VIA_RESTRICT V);
void pause_thread(State *VIA_RESTRICT V);

} // namespace via