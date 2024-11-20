/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "state.h"

namespace via
{

// Main VM function that starts execution
void via_execute(viaState *V);
void via_killthread(viaState *V);
void via_pausethread(viaState *V);

} // namespace via