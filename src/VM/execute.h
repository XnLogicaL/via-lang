/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "state.h"

namespace via
{

// Main VM function that starts execution
void execute(State *VIA_RESTRICT V);
void killthread(State *VIA_RESTRICT V);
void pausethread(State *VIA_RESTRICT V);

} // namespace via