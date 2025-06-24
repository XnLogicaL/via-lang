// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "sbits.h"

namespace via {

uint32_t ubit_2u16tou32(uint16_t high, uint16_t low) {
  return (static_cast<uint32_t>(low) << 16) | high;
}

int32_t ubit_2u16toi32(uint16_t high, uint16_t low) {
  uint32_t unsign = ubit_2u16tou32(high, low);
  return static_cast<int32_t>(unsign);
}

float ubit_2u16tof32(uint16_t high, uint16_t low) {
  uint32_t combined = (static_cast<uint32_t>(low) << 16) | high;
  return std::bit_cast<float>(combined);
}

u16result ubit_u32to2u16(uint32_t data) {
  return {
    .high = static_cast<uint16_t>(data & 0xFFFF),
    .low = static_cast<uint16_t>(data >> 16),
  };
}

} // namespace via
