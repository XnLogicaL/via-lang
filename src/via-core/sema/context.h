// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_CONTEXT_H_
#define VIA_CORE_CONTEXT_H_

#include <via/config.h>
#include <via/types.h>
#include "buffer.h"
#include "memory.h"

namespace via {

namespace sema {

struct SemaContext {
  HeapAllocator alloc;
  Buffer<u64> regs{UINT16_MAX / 8};
};

}  // namespace sema

}  // namespace via

#endif
