/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "shared.h"
#include "api.h"
#include "libutils.h"

namespace via::lib
{

void table_insert(viaState *);
void table_insertat(viaState *);
void table_remove(viaState *);
void table_removeat(viaState *);
void table_contains(viaState *);
void table_concat(viaState *);
void table_clone(viaState *);
void table_deepclone(viaState *);
void table_len(viaState *);
void table_indexof(viaState *);
void table_keys(viaState *);
void table_values(viaState *);
void table_sort(viaState *);
void table_reverse(viaState *);
void table_foreach(viaState *);
void table_map(viaState *);
void table_filter(viaState *);
void table_reduce(viaState *);
void table_merge(viaState *);
void table_slice(viaState *);
void table_clear(viaState *);
void table_compare(viaState *);

} // namespace via::lib