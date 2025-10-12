/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "memory.hpp"
#include <mimalloc.h>

via::ScopedAllocator::ScopedAllocator()
    : m_heap(mi_heap_new())
{}

via::ScopedAllocator::~ScopedAllocator()
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

bool via::ScopedAllocator::owns(void* ptr) noexcept
{
    return m_heap && mi_heap_check_owned((mi_heap_t*) m_heap, ptr);
}

void* via::ScopedAllocator::alloc(size_t size) noexcept
{
    return mi_heap_malloc((mi_heap_t*) m_heap, size);
}

char* via::ScopedAllocator::strdup(const char* str) noexcept
{
    return mi_heap_strdup((mi_heap_t*) m_heap, str);
}

char* via::ScopedAllocator::strndup(const char* str, size_t n) noexcept
{
    return mi_heap_strndup((mi_heap_t*) m_heap, str, n);
}

void via::ScopedAllocator::free(void* ptr)
{
    debug::require(
        owns(ptr),
        std::format(
            "free() called on pointer {:p} not owned by allocator {:p}",
            (const void*) ptr,
            (const void*) this
        )
    );

    auto it = std::find_if(
        m_registry.begin(),
        m_registry.end(),
        [&](const detail::ObjectEntry& e) { return e.ptr == static_cast<void*>(ptr); }
    );

    [[likely]] if (it != m_registry.end()) {
        if (!it->destroyed) {
            it->dtor(it->ptr, it->count);
            it->destroyed = true;
        }
        m_registry.erase(it);
    }
    mi_free(ptr);
}
