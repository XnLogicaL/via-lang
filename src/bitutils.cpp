// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "bitutils.h"

VIA_NAMESPACE_BEGIN

U32 reinterpret_u16_as_u32(U16 high, U16 low) {
    return (static_cast<U32>(low) << 16) | high;
}

I32 reinterpret_u16_as_i32(U16 high, U16 low) {
    U32 unsign = reinterpret_u16_as_u32(high, low);
    return static_cast<I32>(unsign);
}

F32 reinterpret_u16_as_f32(U16 high, U16 low) {
    U32 combined = (static_cast<U32>(low) << 16) | high;
    return std::bit_cast<F32>(combined);
}

U16Result reinterpret_u32_as_2u16(U32 data) {
    return {
        .l = static_cast<U16>(data & 0xFFFF),
        .r = static_cast<U16>(data >> 16),
    };
}

VIA_NAMESPACE_END
