/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "shared.h"
#include "api.h"

namespace via::lib
{

void table_insert(viaState *);
void table_create(viaState *);
void table_remove(viaState *);
void table_foreach(viaState *);

} // namespace via::lib