// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "register.h"

namespace via {

namespace sema {

int alloc_register(SemaContext& ctx) {
  for (u16 i = 0; i < UINT16_MAX; i++) {
    for (u16 j = 0; j < 64; j++) {
      u64* addr = ctx.regs.data + i;
      u64 mask = (1ULL << j);

      if ((*addr & mask) == 0ULL) {
        *addr |= mask;        // mark bit as occupied
        return (i * 64) + j;  // absolute register address
      }
    }
  }

  return -1;
}

void free_register(SemaContext ctx, int reg) {
  u16 word = reg / 64, bit = reg % 64;
  u64 mask = ~(1ULL << bit);
  ctx.regs.data[word] &= mask;  // mark bit as free
}

}  // namespace sema

}  // namespace via
