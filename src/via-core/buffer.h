// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_BUFFER_H_
#define VIA_BUFFER_H_

#include <via/config.h>
#include <via/types.h>
#include "memory.h"

namespace via
{

template <typename T, typename Ator = StdAllocator>
class Buffer
{
 public:
  using value_type = T;
  using size_type = usize;
  using pointer = T*;
  using const_pointer = const T*;
  using iterator = T*;
  using const_iterator = const T*;

 public:
  Buffer() noexcept = default;

  Buffer(size_type sz) : m_data(Ator::template alloc<T>(sz)), m_size(sz)
  {
    m_construct_range(m_data, m_size);
  }

  Buffer(const_iterator begin, const_iterator end)
      : m_size(end - begin), m_data(Ator::template alloc<T>(m_size))
  {
    m_construct_range(m_data, begin, end);
  }

  Buffer(const Buffer& other)
      : m_size(other.m_size), m_data(Ator::template alloc<T>(other.m_size))
  {
    m_construct_range(m_data, other.begin(), other.end());
  }

  Buffer(Buffer&& other) noexcept : m_data(other.m_data), m_size(other.m_size)
  {
    other.m_data = nullptr;
    other.m_size = 0;
  }

  Buffer& operator=(const Buffer& other)
  {
    if (this != &other) {
      m_clear();
      m_resize(other.m_size);
      m_construct_range(m_data, other.begin(), other.end());
    }
    return *this;
  }

  Buffer& operator=(Buffer&& other) noexcept
  {
    if (this != &other) {
      m_clear();
      m_data = other.m_data;
      m_size = other.m_size;
      other.m_data = nullptr;
      other.m_size = 0;
    }
    return *this;
  }

  ~Buffer() { m_clear(); }

  value_type& operator[](size_type i) noexcept { return m_data[i]; }
  const value_type& operator[](size_type i) const noexcept { return m_data[i]; }

 public:
  size_type size() const noexcept { return m_size; }
  pointer data() noexcept { return m_data; }
  const_pointer data() const noexcept { return m_data; }
  iterator begin() noexcept { return m_data; }
  iterator end() noexcept { return m_data + m_size; }
  const_iterator begin() const noexcept { return m_data; }
  const_iterator end() const noexcept { return m_data + m_size; }
  const_iterator cbegin() const noexcept { return m_data; }
  const_iterator cend() const noexcept { return m_data + m_size; }

 private:
  pointer m_data = nullptr;
  size_type m_size = 0;

  template <typename It>
  void m_construct_range(pointer dest, It begin, It end)
  {
    for (pointer d = dest; begin != end; ++d, ++begin) {
      new (d) T(*begin);
    }
  }

  void m_construct_range(pointer dest, size_type count)
  {
    if constexpr (!std::is_trivially_default_constructible_v<T>) {
      for (size_type i = 0; i < count; ++i)
        new (&dest[i]) T();
    }
  }

  void m_clear()
  {
    if (m_data) {
      if constexpr (!std::is_trivially_destructible_v<T>) {
        for (size_type i = 0; i < m_size; ++i)
          m_data[i].~T();
      }
      Ator::template free<T>(m_data);
      m_data = nullptr;
      m_size = 0;
    }
  }

  void m_resize(size_type new_size)
  {
    m_data = Ator::template alloc<T>(new_size);
    m_size = new_size;
    m_construct_range(m_data, m_size);
  }
};

}  // namespace via

#endif
