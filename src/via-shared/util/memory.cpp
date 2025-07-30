// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "memory.h"

namespace via {

void* HeapAllocator::alloc(usize size) {
  return mi_heap_malloc(heap, size);
}

void HeapAllocator::free(void* ptr) {
  mi_free(ptr);
}

}  // namespace via
