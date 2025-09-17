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
#include "support/utility.h"

namespace via {
namespace detail {

template <typename T, typename... Args>
void construct_at(T* dst, Args&&... args)
{
    (void) (new (dst) T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
void construct_range_at(T* dst, usize sz, Args&&... args)
{
    for (usize i = 0; i < sz; i++)
        construct_at(dst + i, std::forward<Args>(args)...);
}

} // namespace detail

struct StdAllocator
{
    template <typename T>
    static T* alloc(usize sz)
    {
        return (T*) std::calloc(sz, sizeof(T));
    }

    template <typename T>
    static void free(T* ptr)
    {
        std::free((void*) ptr);
    }
};

struct MiAllocator
{
    static mi_heap_t* get_allocator()
    {
        static mi_heap_t* alloc = mi_heap_new();
        return alloc;
    }

    template <typename T>
    static T* alloc(usize sz)
    {
        return (T*) mi_heap_calloc(get_allocator(), sz, sizeof(T));
    }

    template <typename T>
    static void free(T* ptr)
    {
        mi_free((void*) ptr);
    }
};

template <typename Alloc = StdAllocator>
class BumpAllocator final
{
  public:
    BumpAllocator(usize sz) :
        m_base(Alloc::template alloc<std::byte>(sz)),
        m_cursor(m_base)
    {}
    ~BumpAllocator() { Alloc::template free<std::byte>(m_base); }

  public:
    template <typename T>
    [[nodiscard]] T* alloc()
    {
        T* ptr = (T*) m_cursor;
        m_cursor += sizeof(T);
        detail::construct_at(ptr);
        return ptr;
    }

    template <typename T>
    [[nodiscard]] T* alloc(usize sz)
    {
        T* ptr = (T*) m_cursor;
        m_cursor += sizeof(T) * sz;
        detail::construct_range_at(ptr, sz);
        return ptr;
    }

    template <typename T, typename... Args>
    [[nodiscard]] T* emplace(Args&&... args)
    {
        T* ptr = (T*) m_cursor;
        m_cursor += sizeof(T);
        detail::construct_at(ptr, std::forward<Args>(args)...);
        return ptr;
    }

    template <typename T, typename... Args>
    [[nodiscard]] T* emplace(usize sz, Args&&... args)
    {
        T* ptr = (T*) m_cursor;
        m_cursor += sizeof(T) * sz;
        detail::construct_range_at(ptr, sz, std::forward<Args>(args)...);
        return ptr;
    }

  private:
    std::byte* m_base;
    std::byte* m_cursor;
};

class Allocator final
{
  public:
    Allocator() = default;
    ~Allocator() { mi_heap_destroy(m_heap); }

    NO_COPY(Allocator);
    NO_MOVE(Allocator);

  public:
    void free(void* ptr) { mi_free(ptr); }
    [[nodiscard]] bool owns(void* ptr) { return mi_heap_check_owned(m_heap, ptr); }
    [[nodiscard]] void* alloc(usize size) { return mi_heap_malloc(m_heap, size); }

    template <typename T>
    [[nodiscard]] T* alloc()
    {
        return (T*) mi_heap_malloc(m_heap, sizeof(T));
    }

    template <typename T>
    [[nodiscard]] T* alloc(usize count)
    {
        return (T*) mi_heap_malloc(m_heap, count * sizeof(T));
    }

    template <typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] T* emplace(Args&&... args)
    {
        T* mem = (T*) mi_heap_malloc(m_heap, sizeof(T));
        detail::construct_at(mem, std::forward<Args>(args)...);
        return mem;
    }

    template <typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] T* emplace(usize count, Args&&... args)
    {
        T* mem = (T*) mi_heap_malloc(m_heap, count * sizeof(T));
        detail::construct_range_at(mem, count, std::forward<Args>(args)...);
        return mem;
    }

    [[nodiscard]] char* strdup(const char* str) { return mi_heap_strdup(m_heap, str); }
    [[nodiscard]] char* strndup(const char* str, usize n) { return mi_heap_strndup(m_heap, str, n); }

  private:
    mi_heap_t* m_heap = mi_heap_new();
};

} // namespace via
