// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"
#include "function.h"

#define VIA_UPV_RESIZE_FACTOR 2

namespace via::impl {

VIA_INLINE void __closure_upvs_resize(TFunction *_Closure)
{
    U32      _Current_size = _Closure->upv_count;
    U32      _New_size     = _Current_size * VIA_UPV_RESIZE_FACTOR;
    TValue **_New_location = new UpValue[_New_size];

    std::memcpy(_New_location, _Closure->upvs, _Closure->upv_count * sizeof(UpValue));

    delete[] _Closure->upvs;
    _Closure->upvs      = _New_location;
    _Closure->upv_count = _New_size;
}

VIA_INLINE bool __closure_upvs_range_check(TFunction *_Closure, SIZE index)
{
    return _Closure->upv_count >= index;
}

VIA_INLINE TFunction *__create_main_function(State *_State)
{
    return new TFunction{
        .ret_addr     = _State->ip,
        .bytecode     = _State->ip,
        .bytecode_len = static_cast<U32>(_State->ibp - _State->ihp),
    };
}

} // namespace via::impl
