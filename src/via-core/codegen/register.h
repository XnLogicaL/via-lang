// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_REGISTER_H_
#define VIA_CORE_REGISTER_H_

#include "common.h"
#include "buffer.h"

namespace via {

struct SemaRegisterState {
  Buffer<u64> buf{UINT16_MAX / 8};
};

int sema_alloc_register(SemaRegisterState& S);
void sema_free_register(SemaRegisterState& S, int reg);

} // namespace via

#endif
