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

  inline HeapBuffer(const size_t size)
    : data(new T[size]),
      cursor(data),
      size(size) {}

  inline ~HeapBuffer() {
    delete[] data;
  }

  inline HeapBuffer(const HeapBuffer<T>& other)
    : data(new T[other.size]),
      cursor(data),
      size(other.size) {
    std::memcpy(data, other.data, size);
  }

  inline HeapBuffer(HeapBuffer<T>&& other)
    : data(other.data),
      cursor(data),
      size(other.size) {
    other.data = NULL;
    other.cursor = NULL;
    other.size = 0;
  }

  HeapBuffer& operator=(const HeapBuffer<T>& other) {
    if (this != &other) {
      delete[] data;
      data = new T[other.size];
      cursor = data;
      size = other.size;
      std::memcpy(data, other.data, size);
    }

    return *this;
  }

  HeapBuffer& operator=(HeapBuffer<T>&& other) {
    if (this != &other) {
      delete[] data;
      data = other.data;
      cursor = data;
      size = other.size;
      other.data = NULL;
      other.cursor = NULL;
      other.size = 0;
    }

    return *this;
  }
};

} // namespace via

#endif
