/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "state.h"

namespace via
{

// Main VM function that starts execution
void execute(RTState *V);
void killthread(RTState *V);
void pausethread(RTState *V);

} // namespace via