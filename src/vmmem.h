// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_MEM_H
#define VIA_MEM_H

#include <memory>
#include <cstring>
#include "common.h"
#include "vmval.h"
#include "vmerr.h"

#define VIA_PAGESIZE      1 << 12
#define VIA_MAXPAGEALLOCS 1 << 7

namespace via {

namespace vm {

struct HeapAllocInfo {
  void* ptr = NULL;
  size_t size = 0;
};

struct HeapPage {
  HeapPage* next;
  size_t used;
  size_t allocs;
  uint8_t data[VIA_PAGESIZE];
  HeapAllocInfo infos[VIA_MAXPAGEALLOCS];
};

struct Heap {
  HeapPage* head;

  Heap();
  ~Heap();

  VIA_NOCOPY(Heap);
  VIA_NOMOVE(Heap);
};

inline constexpr size_t heap_align(size_t value, size_t alignment) {
  return (value + alignment - 1) & ~(alignment - 1);
}

bool heap_checkptr(Heap* H, void* ptr);
void heap_allocpage(Heap* H);
void heap_reset(Heap* H);
void* heap_alloc(Heap* H, size_t bytes, size_t alignment = alignof(max_align_t));
void heap_free(Heap* H, void* ptr);

template<typename T, typename... Args>
  requires std::is_constructible_v<T, Args...>
T* heap_emplace(Heap* H, Args&&... args) {
  void* mem = heap_alloc(H, sizeof(T), alignof(T));
  return new (mem) T(std::forward<Args>(args)...);
}

template<typename T>
void heap_delete(Heap* H, T* obj) {
  if (!heap_checkptr(H, obj)) {
    error_fatal("memory deallocation error: heap_delete called on separately allocated object");
    return;
  }

  obj->~T();
  heap_free(H, obj);
}

} // namespace vm

} // namespace via

#endif
