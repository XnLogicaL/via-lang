// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "semareg.h"

namespace via {

int sema_alloc_register(SemaRegisterState& S) {
  for (u16 i = 0; i < UINT16_MAX; i++) {
    for (u16 j = 0; j < 64; j++) {
      u64* addr = S.buf.data + i;
      u64 mask = (1ULL << j);

      if ((*addr & mask) == 0ULL) {
        *addr |= mask;       // mark bit as occupied
        return (i * 64) + j; // absolute register address
      }
    }
  }

  return -1;
}

void sema_free_register(SemaRegisterState& S, int reg) {
  u16 wrd = reg / 64, bit = reg % 64;
  u64 mask = ~(1ULL << bit);
  S.buf.data[wrd] &= mask; // mark bit as free
}

} // namespace via
