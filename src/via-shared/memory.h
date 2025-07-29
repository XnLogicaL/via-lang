// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SHARED_MEMORY_H_
#define VIA_SHARED_MEMORY_H_

#include "common.h"
#include <mimalloc.h>

namespace via {

struct HeapAllocator {
  mi_heap_t* heap;

  inline explicit HeapAllocator()
    : heap(mi_heap_new()) {}

  inline ~HeapAllocator() {
    mi_heap_destroy(heap);
  }

  VIA_NOCOPY(HeapAllocator);
  VIA_NOMOVE(HeapAllocator);
};

void* heap_alloc(HeapAllocator& heap, const usize size);

template<typename T>
inline T* heap_alloc(HeapAllocator& heap) {
  return (T*)mi_heap_malloc(heap.heap, sizeof(T));
}

template<typename T>
inline T* heap_alloc(HeapAllocator& heap, const usize count) {
  return (T*)mi_heap_malloc(heap.heap, count * sizeof(T));
}

template<typename T, typename... Args>
inline T* heap_emplace(HeapAllocator& heap, Args... args) {
  void* mem = mi_heap_malloc(heap.heap, sizeof(T));
  return new (mem) T(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
inline T* heap_emplace(HeapAllocator& heap, usize count, Args&&... args) {
  T* ptr = (T*)mi_heap_malloc(heap.heap, count * sizeof(T));
  for (usize i = 0; i < count; ++i)
    new (&ptr[i]) T(std::forward<Args>(args)...);
  return ptr;
}

} // namespace via

#endif
