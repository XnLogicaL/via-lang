// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "bit-utility.h"

namespace via {

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

u16_result reinterpret_u32_as_2u16(uint32_t data) {
  return {
    .high = static_cast<uint16_t>(data & 0xFFFF),
    .low = static_cast<uint16_t>(data >> 16),
  };
}

} // namespace via
