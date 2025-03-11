// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "bitutils.h"

VIA_NAMESPACE_BEGIN

u32 reinterpret_u16_as_u32(u16 high, u16 low) {
    return (static_cast<u32>(low) << 16) | high;
}

i32 reinterpret_u16_as_i32(u16 high, u16 low) {
    u32 unsign = reinterpret_u16_as_u32(high, low);
    return static_cast<i32>(unsign);
}

f32 reinterpret_u16_as_f32(u16 high, u16 low) {
    u32 combined = (static_cast<u32>(low) << 16) | high;
    return std::bit_cast<f32>(combined);
}

u16Result reinterpret_u32_as_2u16(u32 data) {
    return {
        .l = static_cast<u16>(data & 0xFFFF),
        .r = static_cast<u16>(data >> 16),
    };
}

VIA_NAMESPACE_END
