// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_RTTYPES_H
#define _VIA_RTTYPES_H

#include "common.h"
#include "state.h"
#include "function.h"
#include "object.h"

VIA_NAMESPACE_BEGIN

VIA_FORCE_INLINE bool check_integer(const TValue& val) {
    return val.type == ValueType::integer;
}

VIA_FORCE_INLINE bool check_floating_point(const TValue& val) {
    return val.type == ValueType::floating_point;
}

VIA_FORCE_INLINE bool check_number(const TValue& val) {
    return check_integer(val) || check_floating_point(val);
}

VIA_FORCE_INLINE bool check_bool(const TValue& val) {
    return val.type == ValueType::boolean;
}

VIA_FORCE_INLINE bool check_nil(const TValue& val) {
    return val.type == ValueType::nil;
}

VIA_FORCE_INLINE bool check_string(const TValue& val) {
    return val.type == ValueType::string;
}

VIA_FORCE_INLINE bool check_table(const TValue& val) {
    return val.type == ValueType::table;
}

VIA_FORCE_INLINE bool check_cfunction(const TValue& val) {
    return val.type == ValueType::cfunction;
}

VIA_FORCE_INLINE bool check_function(const TValue& val) {
    return val.type == ValueType::function;
}

VIA_FORCE_INLINE bool check_callable(const TValue& val) {
    return check_function(val) || check_cfunction(val);
}

VIA_FORCE_INLINE bool check_subscriptable(const TValue& val) {
    return check_table(val) || check_string(val);
}

VIA_NAMESPACE_END

#endif
