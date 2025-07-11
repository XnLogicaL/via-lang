// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "semareg.h"

namespace via {

int sema_alloc_register(SemaRegisterState& S) {
  for (u16 i = 0; i < UINT16_MAX; i++) {
    for (u16 j = 0; j < 64; j++) {
      u64* reg = S.buf.data + i;
      u64 mask = (1ULL << j);

      if ((*reg & mask) == 0) {
        *reg |= mask;        // mark bit as occupied
        return (i * 64) + j; // absolute register address
      }
    }
  }

  return -1;
}

void sema_free_register(SemaRegisterState& S, int reg) {
  u16 word_index = reg / 64;
  u16 bit_index = reg % 64;

  u64 mask = ~(1ULL << bit_index);
  S.buf.data[word_index] &= mask; // mark bit as free
}

} // namespace via
