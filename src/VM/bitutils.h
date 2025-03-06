// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#pragma once

#include "common.h"

namespace via {

U32 reinterpret_u16_as_u32(U16 high, U16 low);
I32 reinterpret_u16_as_i32(U16 high, U16 low);
F32 reinterpret_u16_as_f32(U16 high, U16 low);

} // namespace via
