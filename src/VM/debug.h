/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "api.h"

namespace via
{

void viaD_printregistermap(viaState *, size_t);
void viaD_printargumentstack(viaState *, size_t);
void viaD_printreturnstack(viaState *, size_t);
void viaD_printcallstack(viaState *, size_t);

} // namespace via