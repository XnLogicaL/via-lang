/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <mimalloc.h>
#include <via/config.h>
#include <via/types.h>
#include "utility.h"

namespace via
{

namespace detail
{

template <typename T, typename... Args>
void constructAt(T* dst, Args&&... args)
{
  (void)(new (dst) T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
void constructRangeAt(T* dst, usize sz, Args&&... args)
{
  for (usize i = 0; i < sz; i++)
    constructAt(dst + i, std::forward<Args>(args)...);
}

}  // namespace detail

struct StdAllocator
{
  template <typename T>
  static T* alloc(usize sz)
  {
    return (T*)std::calloc(sz, sizeof(T));
  }

  template <typename T>
  static void free(T* ptr)
  {
    std::free((void*)ptr);
  }
};

struct MiAllocator
{
  static mi_heap_t* getAllocator()
  {
    static mi_heap_t* alloc = mi_heap_new();
    return alloc;
  }

  template <typename T>
  static T* alloc(usize sz)
  {
    return (T*)mi_heap_calloc(getAllocator(), sz, sizeof(T));
  }

  template <typename T>
  static void free(T* ptr)
  {
    mi_free((void*)ptr);
  }
};

template <typename Alloc = StdAllocator>
class BumpAllocator final
{
 public:
  BumpAllocator(usize sz)
      : m_base(Alloc::template alloc<std::byte>(sz)), mCursor(m_base)
  {}
  ~BumpAllocator() { Alloc::template free<std::byte>(m_base); }

 public:
  template <typename T>
  [[nodiscard]] T* alloc()
  {
    T* ptr = (T*)mCursor;
    mCursor += sizeof(T);
    detail::constructAt(ptr);
    return ptr;
  }

  template <typename T>
  [[nodiscard]] T* alloc(usize sz)
  {
    T* ptr = (T*)mCursor;
    mCursor += sizeof(T) * sz;
    detail::constructRangeAt(ptr, sz);
    return ptr;
  }

  template <typename T, typename... Args>
  [[nodiscard]] T* emplace(Args&&... args)
  {
    T* ptr = (T*)mCursor;
    mCursor += sizeof(T);
    detail::constructAt(ptr, std::forward<Args>(args)...);
    return ptr;
  }

  template <typename T, typename... Args>
  [[nodiscard]] T* emplace(usize sz, Args&&... args)
  {
    T* ptr = (T*)mCursor;
    mCursor += sizeof(T) * sz;
    detail::constructRangeAt(ptr, sz, std::forward<Args>(args)...);
    return ptr;
  }

 private:
  std::byte* m_base;
  std::byte* mCursor;
};

class Allocator final
{
 public:
  Allocator() = default;
  ~Allocator() { mi_heap_destroy(mHeap); }

  NO_COPY(Allocator);
  NO_MOVE(Allocator);

 public:
  void free(void* ptr);

  [[nodiscard]] void* alloc(usize size);
  [[nodiscard]] char* strdup(const char* str);
  [[nodiscard]] bool owns(void* ptr) { return mi_heap_check_owned(mHeap, ptr); }

  template <typename T>
  [[nodiscard]] T* alloc()
  {
    return (T*)mi_heap_malloc(mHeap, sizeof(T));
  }

  template <typename T>
  [[nodiscard]] T* alloc(usize count)
  {
    return (T*)mi_heap_malloc(mHeap, count * sizeof(T));
  }

  template <typename T, typename... Args>
    requires std::is_constructible_v<T, Args...>
  [[nodiscard]] T* emplace(Args&&... args)
  {
    T* mem = (T*)mi_heap_malloc(mHeap, sizeof(T));
    detail::constructAt(mem, std::forward<Args>(args)...);
    return mem;
  }

  template <typename T, typename... Args>
    requires std::is_constructible_v<T, Args...>
  [[nodiscard]] T* emplace(usize count, Args&&... args)
  {
    T* mem = (T*)mi_heap_malloc(mHeap, count * sizeof(T));
    detail::constructRangeAt(mem, count, std::forward<Args>(args)...);
    return mem;
  }

 private:
  mi_heap_t* mHeap = mi_heap_new();
};

}  // namespace via
