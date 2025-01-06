/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vltable.h"

namespace via::lib
{

TableKey _get_largest_key(TTable *tbl)
{
    TableKey largest = 0;

    for (auto it : tbl->data)
        if (it.first > largest)
            largest = it.first;

    return largest;
}

void table_insert(RTState *V)
{
    TValue *tbl = getargument(V, 0);
    TValue *val = getargument(V, 1);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));
    LIB_ASSERT(!tbl->val_table->frozen.get(), "Attempt to modify locked table");

    auto data = tbl->val_table->data;
    size_t size = data.size();

    data[size + 1] = *val;

    nativeret(V, 0);
}

void table_insertat(RTState *V)
{
    TValue *tbl = getargument(V, 0);
    TValue *index = getargument(V, 1);
    TValue *val = getargument(V, 2);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));
    LIB_ASSERT(!tbl->val_table->frozen.get(), "Attempt to modify locked table");
    LIB_ASSERT(checknumber(V, *index), ARG_MISMATCH(1, "Number", ENUM_NAME(index->type)));

    TableKey idx = static_cast<TableKey>(index->val_number);
    LIB_ASSERT(idx > 0, "Index must be greater than 0");

    auto &data = tbl->val_table->data;
    TableKey max_key = 0;
    for (const auto &[key, _] : data)
        if (key > max_key)
            max_key = key;

    LIB_ASSERT(idx <= max_key + 1, "Index out of range");

    for (TableKey key = max_key; key >= idx; --key)
        if (data.find(key) != data.end())
            data[key + 1] = data[key];

    data[idx] = *val;

    nativeret(V, 0);
}

void table_remove(RTState *V)
{
    TValue *tbl = getargument(V, 0);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));
    LIB_ASSERT(!tbl->val_table->frozen.get(), "Attempt to modify locked table");

    auto &data = tbl->val_table->data;
    TableKey last_key = _get_largest_key(tbl->val_table);
    TValue last_val = data[last_key];

    data.erase(last_key);
    pushval(V, last_val);
}

void table_removeat(RTState *V)
{
    TValue *tbl = getargument(V, 0);
    TValue *idx = getargument(V, 1);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));
    LIB_ASSERT(checknumber(V, *idx), ARG_MISMATCH(1, "Number", ENUM_NAME(idx->type)));
    LIB_ASSERT(!tbl->val_table->frozen.get(), "Attempt to modify locked table");

    TableKey index = static_cast<TableKey>(idx->val_number);
    auto &data = tbl->val_table->data;
    TValue rem_val = data[index];

    data.erase(index);
    pushval(V, rem_val);
    nativeret(V, 1);
}

void table_contains(RTState *V)
{
    TValue *tbl = getargument(V, 0);
    TValue *val = getargument(V, 1);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));

    auto &data = tbl->val_table->data;
    for (auto it : data)
        if (compare(V, it.second, *val))
        {
            TNumber key = static_cast<TNumber>(it.first);
            pushval(V, stackvalue(V, key));
            return;
        }

    pushval(V, stackvalue(V));
    nativeret(V, 1);
}

void table_concat(RTState *V)
{
    TValue *tbl = getargument(V, 0);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));

    std::string buf;
    auto &data = tbl->val_table->data;

    for (auto it : data)
    {
        TValue it_val = tostring(V, it.second);
        TString *it_string = it_val.val_string;
        buf += std::string(it_string->ptr, it_string->len);
    }

    const char *final_str = dupstring(buf);
    TValue final = stackvalue(V, final_str);

    pushval(V, final);
    nativeret(V, 1);
}

void table_clone(RTState *V)
{
    TValue *tbl = getargument(V, 0);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));

    TTable *original = tbl->val_table;
    TTable *clone = newtable(V);

    auto &data = clone->data;
    for (const auto &it : original->data)
        data[it.first] = it.second;

    TValue final = stackvalue(V, clone);

    pushval(V, final);
    nativeret(V, 1);
}

void table_deepclone(RTState *V)
{
    TValue *tbl = getargument(V, 0);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));

    TTable *original = tbl->val_table;
    TTable *clone = newtable(V);

    auto &data = clone->data;
    for (const auto &it : original->data)
    {
        TableKey key = it.first;
        bool is_table = checktable(V, it.second);

        if (VIA_UNLIKELY(is_table))
        {
            // Push nested table
            pushval(V, it.second);
            // Call self to deep clone the nested table
            table_deepclone(V);
            data[key] = *popval(V);
        }
        else
            data[key] = it.second;
    }

    TValue final = stackvalue(V, clone);

    pushval(V, final);
    nativeret(V, 1);
}

void table_len(RTState *V)
{
    TValue *tbl = getargument(V, 0);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));

    TValue final = len(V, *tbl);
    pushval(V, final);
    nativeret(V, 1);
}

void table_indexof(RTState *V)
{
    TValue *tbl = getargument(V, 0);
    TValue *val = getargument(V, 1);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));

    auto &data = tbl->val_table->data;
    for (const auto &it : data)
        if (compare(V, it.second, *val))
        {
            pushval(V, stackvalue(V, true));
            return;
        }

    pushval(V, stackvalue(V));
    nativeret(V, 1);
}

void table_keys(RTState *V)
{
    TValue *tbl = getargument(V, 0);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));

    auto &data = tbl->val_table->data;
    TTable *keys = newtable(V);
    TValue keys_table = stackvalue(V, keys);

    for (const auto &it : data)
    {
        // TODO: Find a way to un-hash the string so that it's not just some random numbers
        TNumber key_num = static_cast<TNumber>(it.first);
        TValue key_val = stackvalue(V, key_num);

        pusharguments(V, {keys_table, key_val});
        table_insert(V);
    }

    pushval(V, keys_table);
    nativeret(V, 1);
}

void table_values(RTState *V)
{
    TValue *tbl = getargument(V, 0);

    LIB_ASSERT(checktable(V, *tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl->type)));

    auto &data = tbl->val_table->data;
    TTable *values = newtable(V);
    TValue values_table = stackvalue(V, values);

    for (const auto &it : data)
    {
        TValue val = it.second;
        pusharguments(V, {values_table, val});
        table_insert(V);
    }

    pushval(V, values_table);
    nativeret(V, 1);
}

} // namespace via::lib