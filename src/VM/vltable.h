// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "api.h"
#include "libutils.h"

namespace via::lib {

void table_insert(State *);
void table_insertat(State *);
void table_remove(State *);
void table_removeat(State *);
void table_contains(State *);
void table_concat(State *);
void table_clone(State *);
void table_deepclone(State *);
void table_len(State *);
void table_indexof(State *);
void table_keys(State *);
void table_values(State *);
void table_sort(State *);
void table_reverse(State *);
void table_foreach(State *);
void table_map(State *);
void table_filter(State *);
void table_reduce(State *);
void table_merge(State *);
void table_slice(State *);
void table_clear(State *);
void table_compare(State *);
void loadtablelib(State *);

} // namespace via::lib