// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_BITUTILS_H
#define _VIA_BITUTILS_H

#include "common.h"

VIA_NAMESPACE_BEGIN

struct u16Result {
    u16 l;
    u16 r;
};

u32 reinterpret_u16_as_u32(u16 high, u16 low);
i32 reinterpret_u16_as_i32(u16 high, u16 low);
f32 reinterpret_u16_as_f32(u16 high, u16 low);

u16Result reinterpret_u32_as_2u16(u32 data);

VIA_NAMESPACE_END

#endif
