/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vltable.h"

namespace via::lib
{

viaTableKey _get_largest_key(viaTable *tbl)
{
    viaTableKey largest = 0;

    for (auto it : tbl->data)
        if (it.first > largest)
            largest = it.first;

    return largest;
}

void table_insert(viaState *V)
{
    viaValue tbl = via_popargument(V);
    viaValue val = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));
    LIB_ASSERT(!tbl.val_table->frozen.get(), "Attempt to modify locked table");

    auto data = tbl.val_table->data;
    size_t size = data.size();

    data[size + 1] = val;
}

void table_insertat(viaState *V)
{
    viaValue tbl = via_popargument(V);
    viaValue index = via_popargument(V);
    viaValue val = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));
    LIB_ASSERT(!tbl.val_table->frozen.get(), "Attempt to modify locked table");
    LIB_ASSERT(viaT_checknumber(V, index), ARG_MISMATCH(1, "Number", ENUM_NAME(index.type)));

    viaTableKey idx = static_cast<viaTableKey>(index.val_number);
    LIB_ASSERT(idx > 0, "Index must be greater than 0");

    auto &data = tbl.val_table->data;

    viaTableKey max_key = 0;
    for (const auto &[key, _] : data)
        if (key > max_key)
            max_key = key;

    LIB_ASSERT(idx <= max_key + 1, "Index out of range");

    for (viaTableKey key = max_key; key >= idx; --key)
        if (data.find(key) != data.end())
            data[key + 1] = data[key];

    data[idx] = val;
}

void table_remove(viaState *V)
{
    viaValue tbl = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));
    LIB_ASSERT(!tbl.val_table->frozen.get(), "Attempt to modify locked table");

    auto &data = tbl.val_table->data;
    viaTableKey last_key = _get_largest_key(tbl.val_table);
    viaValue last_val = data[last_key];

    data.erase(last_key);
    via_pushreturn(V, last_val);
}

void table_removeat(viaState *V)
{
    viaValue tbl = via_popargument(V);
    viaValue idx = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));
    LIB_ASSERT(viaT_checknumber(V, idx), ARG_MISMATCH(1, "Number", ENUM_NAME(idx.type)));
    LIB_ASSERT(!tbl.val_table->frozen.get(), "Attempt to modify locked table");

    viaTableKey index = static_cast<viaTableKey>(idx.val_number);
    auto &data = tbl.val_table->data;
    viaValue rem_val = data[index];

    data.erase(index);
    via_pushreturn(V, rem_val);
}

void table_contains(viaState *V)
{
    viaValue tbl = via_popargument(V);
    viaValue val = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));

    auto &data = tbl.val_table->data;
    for (auto it : data)
        if (via_compare(V, it.second, val))
        {
            viaNumber key = static_cast<viaNumber>(it.first);
            via_pushreturn(V, viaT_stackvalue(V, key));
            return;
        }

    via_pushreturn(V, viaT_stackvalue(V));
}

void table_concat(viaState *V)
{
    viaValue tbl = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));

    std::string buf;
    auto &data = tbl.val_table->data;

    for (auto it : data)
    {
        viaValue it_val = via_tostring(V, it.second);
        viaString *it_string = it_val.val_string;
        buf += std::string(it_string->ptr, it_string->len);
    }

    viaRawString_t final_str = strdup(buf.c_str());
    viaValue final = viaT_stackvalue(V, final_str);

    via_pushreturn(V, final);
}

void table_clone(viaState *V)
{
    viaValue tbl = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));

    viaTable *original = tbl.val_table;
    viaTable *clone = viaT_newtable(V);

    auto &data = clone->data;
    for (const auto &it : original->data)
        data[it.first] = it.second;

    viaValue final = viaT_stackvalue(V, clone);

    via_pushreturn(V, final);
}


void table_deepclone(viaState *V)
{
    viaValue tbl = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));

    viaTable *original = tbl.val_table;
    viaTable *clone = viaT_newtable(V);

    auto &data = clone->data;
    for (const auto &it : original->data)
    {
        viaTableKey key = it.first;
        bool is_table = viaT_checktable(V, it.second);

        if (VIA_UNLIKELY(is_table))
        {
            // Push nested table
            via_pushargument(V, it.second);
            // Call self to deep clone the nested table
            table_deepclone(V);
            data[key] = via_popreturn(V);
        }
        else
            data[key] = it.second;
    }

    viaValue final = viaT_stackvalue(V, clone);

    via_pushreturn(V, final);
}

void table_len(viaState *V)
{
    viaValue tbl = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));

    viaValue final = via_len(V, tbl);
    via_pushreturn(V, final);
}

void table_indexof(viaState *V)
{
    viaValue tbl = via_popargument(V);
    viaValue val = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));

    auto &data = tbl.val_table->data;
    for (const auto &it : data)
        if (via_compare(V, it.second, val))
        {
            via_pushreturn(V, viaT_stackvalue(V, true));
            return;
        }

    via_pushreturn(V, viaT_stackvalue(V));
}

void table_keys(viaState *V)
{
    viaValue tbl = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));

    auto &data = tbl.val_table->data;
    viaTable *keys = viaT_newtable(V);
    viaValue keys_table = viaT_stackvalue(V, keys);

    for (const auto &it : data)
    {
        // TODO: Find a way to un-hash the string so that it's not just some random numbers
        viaNumber key_num = static_cast<viaNumber>(it.first);
        viaValue key_val = viaT_stackvalue(V, key_num);

        viaL_pusharguments(V, {keys_table, key_val});
        table_insert(V);
    }

    via_pushreturn(V, keys_table);
}

void table_values(viaState *V)
{
    viaValue tbl = via_popargument(V);

    LIB_ASSERT(viaT_checktable(V, tbl), ARG_MISMATCH(0, "Table", ENUM_NAME(tbl.type)));

    auto &data = tbl.val_table->data;
    viaTable *values = viaT_newtable(V);
    viaValue values_table = viaT_stackvalue(V, values);

    for (const auto &it : data)
    {
        viaValue val = it.second;
        viaL_pusharguments(V, {values_table, val});
        table_insert(V);
    }

    via_pushreturn(V, values_table);
}

} // namespace via::lib