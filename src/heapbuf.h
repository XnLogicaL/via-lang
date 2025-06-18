// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HEAPBUF_H
#define VIA_HEAPBUF_H

#include "common.h"

namespace via {

template<typename T>
struct HeapBuffer {
  T* data = NULL;
  mutable T* cursor = NULL;
  size_t size = 0;

  HeapBuffer() = default;
  HeapBuffer(const size_t size);
  ~HeapBuffer();

  VIA_IMPLCOPY(HeapBuffer);
  VIA_IMPLMOVE(HeapBuffer);
};

} // namespace via

#endif
