// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SEMAREG_H
#define VIA_SEMAREG_H

#include "common.h"
#include "buffer.h"

namespace via {

inline constexpr usize kSemaRegisterArraySize = UINT16_MAX / 64;
inline constexpr usize kSemaRegisterArrayBytes = UINT16_MAX / 8;

struct SemaRegisterState {
  Buffer<u64> buf;

  inline explicit SemaRegisterState()
    : buf(kSemaRegisterArraySize) {
    std::memset(buf.data, 0x0, kSemaRegisterArrayBytes);
  }
};

int sema_alloc_register(SemaRegisterState& S);
void sema_free_register(SemaRegisterState& S, int reg);

} // namespace via

#endif
