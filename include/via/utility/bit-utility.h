// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_BITUTILS_H
#define VIA_HAS_HEADER_BITUTILS_H

#include "common.h"

namespace via {

// Unsigned 32-bit integer in the form of two bytes; high and low.
struct u16_result {
  uint16_t high;
  uint16_t low;
};

/**
 * Casts a high 16-bit unsigned integer and a low 16-bit unsigned integer into a 32-bit unsigned
 * integer. Assumes big endian.
 */
uint32_t reinterpret_u16_as_u32(uint16_t high, uint16_t low);

/**
 * Casts a high 16-bit unsigned integer and a low 16-bit unsigned integer into a 32-bit signed
 * integer. Assumes big endian.
 */
int32_t reinterpret_u16_as_i32(uint16_t high, uint16_t low);

/**
 * Casts a high 16-bit unsigned integer and a low 16-bit unsigned integer into a 32-bit floating
 * point. Assumes IEEE-754 floating-point.
 */
float32_t reinterpret_u16_as_f32(uint16_t high, uint16_t low);

// Casts an unsigned 32-bit integer into two unsigned 16-bit integers in the form a struct.
u16_result reinterpret_u32_as_2u16(uint32_t data);

} // namespace via

#endif
