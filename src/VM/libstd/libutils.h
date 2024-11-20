/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "types.h"
#include "api.h"
#include <cstdint>

#define LIB_ASSERT(cond, msg) \
    via_assert(V, (cond), (msg)); \
    if (!(cond)) \
        return;

inline via::VM::viaTableKey __via_make_key(const char *k)
{
    return via::VM::viaTableKey(k);
}

inline via::VM::viaRegister __via_get_arg_register(size_t roffset)
{
    return {via::VM::RegisterType::AR, static_cast<uint8_t>(roffset)};
}

inline via::VM::viaRegister __via_get_ret_register(size_t roffset)
{
    return {via::VM::RegisterType::RR, static_cast<uint8_t>(roffset)};
}

inline via::VM::viaRegister __via_get_self_register()
{
    return {via::VM::RegisterType::SR, 0};
}

inline bool __via_is_nil(const via::VM::viaValue &v)
{
    return v.type == via::VM::viaValueType::Nil;
}

namespace via::VM::LibConstructor
{

inline viaTable *new_lib(viaState *V)
{
    viaTable *T = new viaTable;
    via_settableindex(V, T, __via_make_key("__type"), viaValue("Library"));
    return T;
}

inline void add_method(viaState *V, viaTable *T, const char *k, void (*f)(viaState *))
{
    via_settableindex(V, T, __via_make_key(k), viaValue(f));
}

inline void add_member(viaState *V, viaTable *T, const char *k, viaValue v)
{
    via_settableindex(V, T, __via_make_key(k), v);
}

inline void seal(viaState *V, viaTable *T)
{
    via_freeze(V, T);
}

} // namespace via::VM::LibConstructor
