/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
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
    size_t count;
    bool destroyed = false;
    void (*dtor)(void*, size_t);
};

using ObRegistry = std::vector<ObjectEntry>;

template <typename T, typename... Args>
[[gnu::hot]] inline void __construct_at(ObRegistry* registry, T* dst, Args&&... args)
{
    ::new (static_cast<void*>(dst)) T(std::forward<Args>(args)...);
    registry->push_back({
        .ptr = dst,
        .count = 1,
        .dtor = [](void* ptr, size_t) { ((T*) ptr)->~T(); },
    });
}

template <typename T, typename... Args>
[[gnu::hot]] inline void
__construct_range_at(ObRegistry* registry, T* dst, size_t count, Args&&... args)
{
    for (size_t i = 0; i < count; ++i) {
        T* ptr = dst + i;
        ::new (static_cast<void*>(ptr)) T(std::forward<Args>(args)...);
    }

    registry->push_back({
        .ptr = static_cast<void*>(dst),
        .count = count,
        .dtor =
            [](void* base, size_t count) noexcept {
                T* start = static_cast<T*>(base);
                for (size_t i = count; i-- > 0;) {
                    (&start[i])->~T();
                }
            },
    });
}

inline void __destroy_registry(ObRegistry* registry) noexcept
{
    for (auto it = registry->rbegin(); it != registry->rend(); ++it) {
        if (!it->destroyed) {
            it->dtor(it->ptr, it->count);
            it->destroyed = true;
        }
    }

    registry->clear();
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
          m_cursor(m_base),
          m_end(m_base + size)
    {}

    ~BumpAllocator()
    {
        detail::__destroy_registry(&m_registry);

        if (m_base) {
            Alloc::template free<std::byte>(m_base);
            m_base = nullptr;
            m_cursor = nullptr;
            m_end = nullptr;
        }
    }

    NO_COPY(BumpAllocator);
    NO_MOVE(BumpAllocator);

  public:
    [[nodiscard]] inline void*
    alloc(size_t size, size_t align = alignof(std::max_align_t))
    {
        std::uintptr_t cur = reinterpret_cast<std::uintptr_t>(m_cursor);
        std::uintptr_t aligned =
            (cur + (align - 1)) & ~(static_cast<std::uintptr_t>(align) - 1);
        std::byte* out = reinterpret_cast<std::byte*>(aligned);

        debug::require(out + size <= m_end, "BumpAllocator overflow");
        m_cursor = out + size;
        return out;
    }

    template <typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] inline T* emplace(Args&&... args)
    {
        T* ptr = (T*) alloc(sizeof(T), alignof(T));
        detail::__construct_at(&m_registry, ptr, std::forward<Args>(args)...);
        return ptr;
    }

    template <typename T, typename... Args>
        requires std::is_constructible_v<T, Args...>
    [[nodiscard]] inline T* emplace_array(size_t count, Args&&... args)
    {
        std::byte* block = (std::byte*) alloc(sizeof(T) * count, alignof(T));
        T* ptr = reinterpret_cast<T*>(block);
        detail::__construct_range_at(
            &m_registry,
            ptr,
            count,
            std::forward<Args>(args)...
        );
        return ptr;
    }

  private:
    std::byte* m_base = nullptr;
    std::byte* m_cursor = nullptr;
    std::byte* m_end = nullptr;
    detail::ObRegistry m_registry;
};

class ScopedAllocator final
{
  public:
    ScopedAllocator()
        : m_heap(mi_heap_new())
    {}

    ~ScopedAllocator()
    {
        detail::__destroy_registry(&m_registry);

        if (m_heap) {
            // TODO: Uncomment when fixed
            // Causes ASan crash due to an internal bug in mimalloc
            // See: https://github.com/microsoft/mimalloc/issues/1146
            //
            // mi_heap_destroy(m_heap);
            m_heap = nullptr;
        }
    }

    NO_COPY(ScopedAllocator);
    NO_MOVE(ScopedAllocator);

  public:
    [[nodiscard]] inline bool owns(void* ptr) noexcept
    {
        return m_heap && mi_heap_check_owned(m_heap, ptr);
    }

    [[nodiscard]] inline void* alloc(size_t size) noexcept
    {
        return mi_heap_malloc(m_heap, size);
    }

    template <typename T>
    inline void free(T* ptr) noexcept(std::is_nothrow_destructible_v<T>)
    {
        debug::require(owns(ptr), "free() called on ptr not owned by allocator");

        auto it = std::find_if(
            m_registry.begin(),
            m_registry.end(),
            [&](const detail::ObjectEntry& e) { return e.ptr == static_cast<void*>(ptr); }
        );

        if (it != m_registry.end()) {
            if (!it->destroyed) {
                it->dtor(it->ptr, it->count);
                it->destroyed = true;
            }

            m_registry.erase(it);
        }
        else {
            debug::require(
                false,
                "free() called for pointer that wasn't registered by "
                "emplace()/emplace_array()"
            );
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
    mi_heap_t* m_heap;
    detail::ObRegistry m_registry;
};

} // namespace via
