// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "bit-utility.h"

VIA_NAMESPACE_BEGIN

uint32_t reinterpret_u16_as_u32(uint16_t high, uint16_t low) {
  return (static_cast<uint32_t>(low) << 16) | high;
}

int32_t reinterpret_u16_as_i32(uint16_t high, uint16_t low) {
  uint32_t unsign = reinterpret_u16_as_u32(high, low);
  return static_cast<int32_t>(unsign);
}

float32_t reinterpret_u16_as_f32(uint16_t high, uint16_t low) {
  uint32_t combined = (static_cast<uint32_t>(low) << 16) | high;
  return std::bit_cast<float32_t>(combined);
}

u16Result reinterpret_u32_as_2u16(uint32_t data) {
  return {
      .l = static_cast<uint16_t>(data & 0xFFFF),
      .r = static_cast<uint16_t>(data >> 16),
  };
}

VIA_NAMESPACE_END
