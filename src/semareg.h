// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SEMAREG_H
#define VIA_SEMAREG_H

#include "common.h"
#include "heapbuf.h"

namespace via {

struct SemaRegisterState {
  HeapBuffer<u64> buf{1024};
};

int sema_alloc_register(SemaRegisterState& S);
void sema_free_register(SemaRegisterState& S, int reg);

} // namespace via

#endif
