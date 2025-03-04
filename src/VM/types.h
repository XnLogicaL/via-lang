// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "state.h"
#include "modifiable_once.h"
#include "function.h"
#include "object.h"

namespace via {

VIA_FORCEINLINE U32 hash_string(const char *str)
{
    static const constexpr U32 BASE = 31;
    static const constexpr U32 MOD = 0xFFFFFFFF;

    U32 hash = 0;
    while (char chr = *str++) {
        hash = (hash * BASE + static_cast<U32>(chr)) % MOD;
    }

    return hash;
}

VIA_FORCEINLINE bool check_integer(const TValue &val)
{
    return val.type == ValueType::integer;
}

VIA_FORCEINLINE bool check_floating_point(const TValue &val)
{
    return val.type == ValueType::floating_point;
}

VIA_FORCEINLINE bool check_number(const TValue &val)
{
    return check_integer(val) || check_floating_point(val);
}

VIA_FORCEINLINE bool check_bool(const TValue &val)
{
    return val.type == ValueType::boolean;
}

VIA_FORCEINLINE bool check_nil(const TValue &val)
{
    return val.type == ValueType::nil;
}

VIA_FORCEINLINE bool check_string(const TValue &val)
{
    return val.type == ValueType::string;
}

VIA_FORCEINLINE bool check_table(const TValue &val)
{
    return val.type == ValueType::table;
}

VIA_FORCEINLINE bool check_cfunction(const TValue &val)
{
    return val.type == ValueType::cfunction;
}

VIA_FORCEINLINE bool check_function(const TValue &val)
{
    return val.type == ValueType::function;
}

VIA_FORCEINLINE bool check_callable(const TValue &val)
{
    return check_function(val) || check_cfunction(val);
}

VIA_FORCEINLINE bool check_subscriptable(const TValue &val)
{
    return check_table(val) || check_string(val);
}

} // namespace via
