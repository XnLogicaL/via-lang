// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "semareg.h"

namespace via {

int sema_alloc_register(const SemaRegisterState& S) {
  for (uint16_t i = 0; i < UINT16_MAX; i++) {
    for (uint16_t j = 0; j < 64; j++) {
      uint64_t* reg = S.buf.data + i;
      uint64_t mask = (1ULL << j);

      if ((*reg & mask) == 0) {
        *reg |= mask;        // mark bit as occupied
        return (i * 64) + j; // absolute register address
      }
    }
  }

  return -1;
}

void sema_free_register(SemaRegisterState& S, int reg) {
  uint16_t word_index = reg / 64;
  uint16_t bit_index = reg % 64;

  uint64_t mask = (0ULL << bit_index);
  S.buf.data[word_index] |= mask; // mark bit as free
}

} // namespace via
