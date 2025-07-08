// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_MEM_H
#define VIA_MEM_H

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

inline void* heap_alloc(HeapAllocator& heap, const size_t size) {
  return mi_heap_malloc(heap.heap, size);
}

template<typename T>
inline T* heap_alloc(HeapAllocator& heap) {
  return (T*)mi_heap_malloc(heap.heap, sizeof(T));
}

template<typename T>
inline T* heap_alloc(HeapAllocator& heap, const size_t count) {
  return (T*)mi_heap_malloc(heap.heap, count * sizeof(T));
}

template<typename T, typename... Args>
inline T* heap_emplace(HeapAllocator& heap, Args... args) {
  void* mem = mi_heap_malloc(heap.heap, sizeof(T));
  return new (mem) T(std::forward<Args>(args)...);
}

template<typename T, typename... Args>
inline T* heap_emplace_array(HeapAllocator& heap, size_t count, Args&&... args) {
  T* ptr = (T*)mi_heap_malloc(heap.heap, count * sizeof(T));
  for (size_t i = 0; i < count; ++i)
    new (&ptr[i]) T(std::forward<Args>(args)...);
  return ptr;
}

} // namespace via

#endif
