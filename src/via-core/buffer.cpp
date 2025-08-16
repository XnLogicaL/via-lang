// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "buffer.h"

namespace via {

template <typename T, const Allocator<T> A, const Deleter<T> D>
void Buffer<T, A, D>::reset_cursor() const {
  this->cursor = data;
}

}  // namespace via
