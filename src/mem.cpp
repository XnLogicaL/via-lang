// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "mem.h"

namespace via {

void* heap_alloc(HeapAllocator& heap, const size_t size) {
  return mi_heap_malloc(heap.heap, size);
}

} // namespace via
