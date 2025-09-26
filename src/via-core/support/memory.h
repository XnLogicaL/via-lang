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
#include <mimalloc/internal.h>
#include <vector>
#include <via/config.h>
#include <via/types.h>
#include "debug.h"
#include "utility.h"

namespace via {
namespace detail {

struct ObjectEntry
{
    void* ptr;
    void (*dtor)(void*);
    inline void operator()() const noexcept { dtor(ptr); }
};

using ObjectRegistry = std::vector<ObjectEntry>;

template <typename T, typename... Args>
[[gnu::hot]] inline void __construct_at(ObjectRegistry* registry, T* dst, Args&&... args)
{
    new (dst) T(std::forward<Args>(args)...);
    registry->push_back({.ptr = dst, .dtor = [](void* ptr) { ((T*) ptr)->~T(); }});
}

template <typename T, typename... Args>
[[gnu::hot]] inline void
__construct_range_at(ObjectRegistry* registry, T* dst, size_t size, Args&&... args)
{
    for (T* ptr = dst; ptr < dst + size; ptr++) {
        __construct_at(registry, ptr, std::forward<Args>(args)...);
        registry->push_back({.ptr = ptr, .dtor = [](void* ptr) { ((T*) ptr)->~T(); }});
    }
}

} // namespace detail

struct DefaultAllocator
{
    template <typename T>
    static T* alloc(size_t size)
    {
        return (T*) std::malloc(size * sizeof(T));
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
    static T* alloc(size_t size)
    {
        return (T*) mi_heap_calloc(get_allocator(), size, sizeof(T));
    }

    template <typename T>
    static void free(T* ptr)
    {
        mi_free((void*) ptr);
    }
};

template <typename Alloc = DefaultAllocator>
class BumpAllocator final
{
  public:
    BumpAllocator(size_t size)
        : m_base(Alloc::template alloc<std::byte>(size)),
          m_cursor(m_base)
    {}
    ~BumpAllocator() { Alloc::template free<std::byte>(m_base); }

  public:
    [[nodiscard]] inline void* alloc(size_t size)
    {
        void* ptr = m_cursor;
        m_cursor += size;
        return ptr;
    }

    template <typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] inline T* emplace(Args&&... args)
    {
        T* ptr = (T*) m_cursor;
        m_cursor += sizeof(T);
        detail::__construct_at(&m_registry, ptr, std::forward<Args>(args)...);
        return ptr;
    }

    template <typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] inline T* emplace_array(size_t size, Args&&... args)
    {
        T* ptr = (T*) m_cursor;
        m_cursor += sizeof(T) * size;
        detail::__construct_range_at(&m_registry, ptr, size, std::forward<Args>(args)...);
        return ptr;
    }

  private:
    std::byte* m_base;
    std::byte* m_cursor;
    detail::ObjectRegistry m_registry;
};

class ScopedAllocator final
{
  public:
    ScopedAllocator() = default;
    ~ScopedAllocator()
    {
        for (const auto& entry: m_registry) {
            entry();
        }
        mi_heap_destroy(m_heap);
    }

    NO_COPY(ScopedAllocator);
    NO_MOVE(ScopedAllocator);

    bool operator==(const ScopedAllocator& other) { return m_heap == other.m_heap; }
    bool operator!=(const ScopedAllocator& other) { return m_heap != other.m_heap; }

  public:
    [[nodiscard]] inline bool owns(void* ptr) noexcept
    {
        return mi_heap_check_owned(m_heap, ptr);
    }

    [[nodiscard]] inline void* alloc(size_t size) noexcept
    {
        return mi_heap_malloc(m_heap, size);
    }

    template <typename T>
    inline void free(T* ptr) noexcept(std::is_nothrow_destructible_v<T>)
    {
        debug::require(owns(ptr), "free() called on ptr not owned by allocator");
        if constexpr (std::is_destructible_v<T>) {
            ptr->~T();
        }
        mi_free(ptr);
    }

    template <typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] inline T* emplace(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        T* buffer = (T*) alloc(sizeof(T));
        detail::__construct_at(&m_registry, buffer, std::forward<Args>(args)...);
        return buffer;
    }

    template <typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] inline T* emplace_array(size_t count, Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        T* buffer = (T*) alloc(count * sizeof(T));
        detail::__construct_range_at(
            &m_registry,
            buffer,
            count,
            std::forward<Args>(args)...
        );
        return buffer;
    }

    [[nodiscard]] inline char* strdup(const char* str) noexcept
    {
        return mi_heap_strdup(m_heap, str);
    }
    [[nodiscard]] inline char* strndup(const char* str, size_t n) noexcept
    {
        return mi_heap_strndup(m_heap, str, n);
    }

  private:
    mi_heap_t* m_heap = mi_heap_new();
    detail::ObjectRegistry m_registry;
};

} // namespace via
