// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "sema.h"

namespace via {

namespace sema {

char* alloc_string(ArenaAllocator& allocator, size_t len) {
  return (char*)allocator.alloc_bytes(len + 1);
}

char* alloc_string(ArenaAllocator& allocator, const char* str) {
  char* new_str = (char*)allocator.alloc_bytes(std::strlen(str) + 1);
  std::strcpy(new_str, str);
  return new_str;
}

char* alloc_string(ArenaAllocator& allocator, const std::string& str) {
  char* new_str = (char*)allocator.alloc_bytes(str.length() + 1);
  std::strcpy(new_str, str.c_str());
  return new_str;
}

} // namespace sema

} // namespace via
