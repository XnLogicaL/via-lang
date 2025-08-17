// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "memory.h"
#include <cstring>

namespace via {

[[nodiscard]] void* Allocator::alloc(usize size) {
  return mi_heap_malloc(m_heap, size);
}

[[nodiscard]] char* Allocator::strdup(const char* str) {
  usize len = strlen(str);
  char* buf = alloc<char>(len + 1);
  memcpy(buf, str, len + 1);
  return buf;
}

void Allocator::free(void* ptr) {
  mi_free(ptr);
}

}  // namespace via
