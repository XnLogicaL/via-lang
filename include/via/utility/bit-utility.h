// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_BITUTILS_H
#define VIA_HAS_HEADER_BITUTILS_H

#include "common.h"

namespace via {

struct u16_result {
  uint16_t l;
  uint16_t r;
};

uint32_t reinterpret_u16_as_u32(uint16_t high, uint16_t low);
int32_t reinterpret_u16_as_i32(uint16_t high, uint16_t low);
float32_t reinterpret_u16_as_f32(uint16_t high, uint16_t low);

u16_result reinterpret_u32_as_2u16(uint32_t data);

} // namespace via

#endif
