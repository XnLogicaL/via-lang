/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "api.h"

namespace via
{

void dbgprintregistermap(State *, size_t);
void dbgprintargumentstack(State *, size_t);
void dbgprintreturnstack(State *, size_t);
void dbgprintcallstack(State *, size_t);

} // namespace via