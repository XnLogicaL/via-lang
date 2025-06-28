// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HEAPBUF_H
#define VIA_HEAPBUF_H

#include <common/common.h>

namespace via {

template<typename T>
struct HeapBuffer {
  T* data = NULL;
  mutable T* cursor = NULL;
  size_t size = 0;

  // clang-format off
  inline T* begin() { return data; }
  inline T* end() { return data + size * sizeof(T); }

  inline const T* begin() const { return data; }
  inline const T* end() const { return data + size * sizeof(T); }
  // clang-format on

  inline HeapBuffer() = default;
  inline HeapBuffer(const size_t size)
    : data(new T[size]),
      cursor(data),
      size(size) {}

  inline HeapBuffer(T* begin, T* end) {
    size_t offset = end - begin;
    size = offset / sizeof(T);
    data = new T[size];
    cursor = data;

    std::memcpy(data, begin, offset);
  }

  inline ~HeapBuffer() {
    delete[] data;
  }

  inline HeapBuffer(const HeapBuffer& other)
    : data(new T[other.size]),
      cursor(data),
      size(other.size) {
    std::memcpy(data, other.data, size);
  }

  inline HeapBuffer(HeapBuffer&& other)
    : data(other.data),
      cursor(data),
      size(other.size) {
    other.data = NULL;
    other.cursor = NULL;
    other.size = 0;
  }

  inline HeapBuffer& operator=(const HeapBuffer& other) {
    if (this != &other) {
      delete[] data;

      data = new T[other.size];
      cursor = data;
      size = other.size;

      std::memcpy(data, other.data, size);
    }

    return *this;
  }

  inline HeapBuffer& operator=(HeapBuffer&& other) {
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
