// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "heapbuf.h"

#include <cstddef>
#include <cstring>

namespace via {

template<typename T>
HeapBuffer<T>::HeapBuffer(const size_t size)
  : data(new T[size]),
    cursor(data),
    size(size) {}

template<typename T>
HeapBuffer<T>::~HeapBuffer<T>() {
  delete[] data;
}

template<typename T>
HeapBuffer<T>::HeapBuffer(const HeapBuffer<T>& other)
  : data(new T[other.size]),
    cursor(data),
    size(other.size) {
  std::memcpy(data, other.data, size);
}

template<typename T>
HeapBuffer<T>::HeapBuffer(HeapBuffer<T>&& other)
  : data(other.data),
    cursor(data),
    size(other.size) {
  other.data = NULL;
  other.cursor = NULL;
  other.size = 0;
}

template<typename T>
HeapBuffer<T>& HeapBuffer<T>::operator=(const HeapBuffer<T>& other) {
  if (this != &other) {
    delete[] data;
    data = new T[other.size];
    cursor = data;
    size = other.size;
    std::memcpy(data, other.data, size);
  }

  return *this;
}

template<typename T>
HeapBuffer<T>& HeapBuffer<T>::operator=(HeapBuffer<T>&& other) {
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

} // namespace via
