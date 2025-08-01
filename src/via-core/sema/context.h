// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_CONTEXT_H_
#define VIA_CORE_CONTEXT_H_

#include <util/buffer.h>
#include <util/memory.h>
#include <via/config.h>

namespace via {

namespace core {

namespace sema {

struct SemaContext {
  HeapAllocator alloc;
  Buffer<u64> regs{UINT16_MAX / 8};
};

}  // namespace sema

}  // namespace core

}  // namespace via

#endif
