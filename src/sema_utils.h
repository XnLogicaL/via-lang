// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_SEMA_UTILS_H
#define VIA_HAS_HEADER_SEMA_UTILS_H

#include "common.h"
#include <arena.h>

namespace via {

namespace sema {

char* alloc_string(ArenaAllocator& allocator, size_t len);
char* alloc_string(ArenaAllocator& allocator, const char* str);
char* alloc_string(ArenaAllocator& allocator, const std::string& str);

template<typename T>
T* alloc_array(ArenaAllocator& allocator, size_t len) {
  return (T*)allocator.alloc_bytes(sizeof(T) * len);
}

template<typename T>
T* alloc_array(ArenaAllocator& allocator, const std::vector<T>& vec) {
  T* ptr = (T*)allocator.alloc_bytes(sizeof(T) * vec.size());
  std::memcpy(ptr, vec.data(), vec.size());
  return ptr;
}

} // namespace sema

} // namespace via

#endif
