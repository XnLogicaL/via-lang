// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_BITS_H
#define VIA_BITS_H

#include <common/common.h>

namespace via {

// Unsigned 32-bit Int in the form of two bytes; high and low.
struct u16result {
  uint16_t high;
  uint16_t low;
};

/**
 * Casts a high 16-bit unsigned Int and a low 16-bit unsigned Int into a 32-bit unsigned
 * Int. Assumes big endian.
 */
uint32_t ubit_2u16tou32(uint16_t high, uint16_t low);

/**
 * Casts a high 16-bit unsigned Int and a low 16-bit unsigned Int into a 32-bit signed
 * Int. Assumes big endian.
 */
int32_t ubit_2u16toi32(uint16_t high, uint16_t low);

/**
 * Casts a high 16-bit unsigned Int and a low 16-bit unsigned Int into a 32-bit floating
 * point.
 */
float ubit_2u16tof32(uint16_t high, uint16_t low);

// Casts an unsigned 32-bit Int into two unsigned 16-bit integers in the form a struct.
u16result ubit_u32to2u16(uint32_t data);

} // namespace via

#endif
