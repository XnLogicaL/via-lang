/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vltable.h"

namespace via::lib
{

TableKey _get_largest_key(TTable tbl)
{
    TableKey largest = 0;

    for (auto &it : tbl.data)
        if (it.first > largest)
            largest = it.first;

    return largest;
}

void table_insert(RTState *) {}

void table_insertat(RTState *) {}

void table_remove(RTState *) {}

void table_removeat(RTState *) {}

void table_contains(RTState *) {}

void table_concat(RTState *) {}

void table_clone(RTState *) {}

void table_deepclone(RTState *) {}

void table_len(RTState *) {}

void table_indexof(RTState *) {}

void table_keys(RTState *) {}

void table_values(RTState *) {}

void loadtablelib(RTState *) {}

} // namespace via::lib