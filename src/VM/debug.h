/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "api.h"

namespace via
{

void dbgprintregistermap(RTState *, size_t);
void dbgprintargumentstack(RTState *, size_t);
void dbgprintreturnstack(RTState *, size_t);
void dbgprintcallstack(RTState *, size_t);

} // namespace via