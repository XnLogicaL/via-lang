// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SHARED_MEMORY_H_
#define VIA_SHARED_MEMORY_H_

#include <mimalloc.h>
#include "via/config.h"

namespace via {

class HeapAllocator {
 public:
  void* alloc(usize size);
  void free(void* ptr);

  template <typename T>
  inline T* alloc() {
    return (T*)mi_heap_malloc(heap, sizeof(T));
  }

  template <typename T>
  inline T* alloc(const usize count) {
    return (T*)mi_heap_malloc(heap, count * sizeof(T));
  }

  template <typename T, typename... Args>
  inline T* emplace(Args... args) {
    void* mem = mi_heap_malloc(heap, sizeof(T));
    return new (mem) T(std::forward<Args>(args)...);
  }

  template <typename T, typename... Args>
  inline T* emplace(usize count, Args&&... args) {
    T* ptr = (T*)mi_heap_malloc(heap, count * sizeof(T));
    for (usize i = 0; i < count; ++i)
      new (&ptr[i]) T(std::forward<Args>(args)...);
    return ptr;
  }

  HeapAllocator() = default;
  ~HeapAllocator() { mi_heap_destroy(heap); }

  VIA_NOCOPY(HeapAllocator);
  VIA_NOMOVE(HeapAllocator);

 private:
  mi_heap_t* heap = mi_heap_new();
};

}  // namespace via

#endif
