// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_BUFVIEW_H
#define VIA_BUFVIEW_H

#include "common.h"

namespace via {

template<typename T>
struct BufferView {
  T* data = NULL;
  mutable T* cursor = NULL;
  size_t size = 0;

  BufferView() = default;

  inline BufferView(T* const data, const size_t size)
    : data(data),
      cursor(data),
      size(size) {}
};

} // namespace via

#endif
