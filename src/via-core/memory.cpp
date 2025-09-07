/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "memory.h"
#include <cstring>

namespace via
{

[[nodiscard]] void* Allocator::alloc(usize size)
{
  return mi_heap_malloc(mHeap, size);
}

[[nodiscard]] char* Allocator::strdup(const char* str)
{
  usize len = strlen(str);
  char* buf = alloc<char>(len + 1);
  memcpy(buf, str, len + 1);
  return buf;
}

void Allocator::free(void* ptr)
{
  mi_free(ptr);
}

}  // namespace via
