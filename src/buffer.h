// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HEAPBUF_H
#define VIA_HEAPBUF_H

#include "common.h"

namespace via {

template<typename T>
using Allocator = T* (*)(usize);

template<typename T>
using Deleter = void (*)(T*);

namespace detail {

template<typename T>
inline T* calloc(usize size) noexcept {
  return (T*)::calloc(size, sizeof(T));
}

template<typename T>
inline void free(T* ptr) noexcept {
  ::free((void*)ptr);
}

} // namespace detail

template<typename T, Allocator<T> At = detail::calloc, Deleter<T> Dt = detail::free>
struct Buffer {
  T* data = NULL;
  mutable T* cursor = NULL;
  usize size = 0;

  // clang-format off
  inline T* begin() { return data; }
  inline T* end() { return data + size * sizeof(T); }

  inline const T* begin() const { return data; }
  inline const T* end() const { return data + size * sizeof(T); }
  // clang-format on

  inline Buffer() = default;
  inline Buffer(const usize size)
    : data(At(size)),
      cursor(data),
      size(size) {}

  inline Buffer(const T* begin, const T* end) {
    usize offset = end - begin;
    size = offset / sizeof(T);
    data = At(size);
    cursor = data;

    std::memcpy(data, begin, offset);
  }

  inline ~Buffer() {
    Dt(data);
  }

  inline Buffer(const Buffer& other)
    : data(At(other.size)),
      cursor(data),
      size(other.size) {
    std::memcpy(data, other.data, size);
  }

  inline Buffer(Buffer&& other)
    : data(other.data),
      cursor(data),
      size(other.size) {
    other.data = NULL;
    other.cursor = NULL;
    other.size = 0;
  }

  inline Buffer& operator=(const Buffer& other) {
    if (this != &other) {
      Dt(data);

      data = At(other.size);
      cursor = data;
      size = other.size;

      std::memcpy(data, other.data, size);
    }

    return *this;
  }

  inline Buffer& operator=(Buffer&& other) {
    if (this != &other) {
      Dt(data);

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
