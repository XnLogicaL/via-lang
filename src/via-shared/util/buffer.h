// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_BUFFER_H_
#define VIA_BUFFER_H_

#include <cstring>
#include "via/config.h"

namespace via {

template <typename T>
using Allocator = T* (*)(usize);

template <typename T>
using Deleter = void (*)(T*);

namespace detail {

template <typename T>
inline T* std_calloc(usize size) noexcept {
  return (T*)std::calloc(size, sizeof(T));
}

template <typename T>
inline void std_free(T* ptr) noexcept {
  std::free((void*)ptr);
}

}  // namespace detail

// clang-format off
template<
  typename T,
  const Allocator<T> Alloc = detail::std_calloc,
  const Deleter<T> Free = detail::std_free
>  // clang-format on
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

  constexpr Buffer() = default;
  constexpr Buffer(const usize size)
      : data(Alloc(size)), cursor(data), size(size) {}

  constexpr Buffer(const T* begin, const T* end) {
    usize offset = end - begin;
    size = offset / sizeof(T);
    data = Alloc(size);
    cursor = data;

    std::memcpy(data, begin, offset);
  }

  ~Buffer() { Free(data); }

  constexpr Buffer(const Buffer& other)
      : data(Alloc(other.size)), cursor(data), size(other.size) {
    std::memcpy(data, other.data, size);
  }

  constexpr Buffer(Buffer&& other)
      : data(other.data), cursor(data), size(other.size) {
    other.data = NULL;
    other.cursor = NULL;
    other.size = 0;
  }

  Buffer& operator=(const Buffer& other) {
    if (this != &other) {
      Free(data);

      data = Alloc(other.size);
      cursor = data;
      size = other.size;

      std::memcpy(data, other.data, size);
    }

    return *this;
  }

  Buffer& operator=(Buffer&& other) {
    if (this != &other) {
      Free(data);

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

}  // namespace via

#endif
