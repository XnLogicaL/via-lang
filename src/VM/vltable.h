/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "shared.h"
#include "api.h"
#include "libutils.h"

namespace via::lib
{

void table_insert(RTState *);
void table_insertat(RTState *);
void table_remove(RTState *);
void table_removeat(RTState *);
void table_contains(RTState *);
void table_concat(RTState *);
void table_clone(RTState *);
void table_deepclone(RTState *);
void table_len(RTState *);
void table_indexof(RTState *);
void table_keys(RTState *);
void table_values(RTState *);
void table_sort(RTState *);
void table_reverse(RTState *);
void table_foreach(RTState *);
void table_map(RTState *);
void table_filter(RTState *);
void table_reduce(RTState *);
void table_merge(RTState *);
void table_slice(RTState *);
void table_clear(RTState *);
void table_compare(RTState *);

} // namespace via::lib