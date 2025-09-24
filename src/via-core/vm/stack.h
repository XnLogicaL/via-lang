/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "support/memory.h"

namespace via {
namespace config {
namespace vm {

VIA_CONSTANT size_t STACK_SIZE = 8192;

}
} // namespace config

template <typename T>
class Stack final
{
  public:
    explicit Stack(ScopedAllocator& alloc)
        : m_bp(alloc.emplace_array<T>(config::vm::STACK_SIZE)),
          m_sp(m_bp)
    {}

    inline size_t size() const { return static_cast<size_t>(m_sp - m_bp); }
    inline size_t capacity() const { return config::vm::STACK_SIZE; }
    inline bool empty() const { return m_sp == m_bp; }

    inline void push(T val)
    {
        debug::require(size() < capacity(), "stack overflow");
        *(m_sp++) = val;
    }

    inline T pop()
    {
        debug::require(!empty(), "stack underflow");
        return *(--m_sp);
    }

    inline T& top()
    {
        debug::require(!empty(), "stack underflow");
        return *(m_sp - 1);
    }

    inline const T& top() const
    {
        debug::require(!empty(), "stack underflow");
        return *(m_sp - 1);
    }

    inline T& at(size_t idx) { return m_bp[idx]; }
    inline const T& at(size_t idx) const { return m_bp[idx]; }

    inline T* begin() { return m_bp; }
    inline const T* begin() const { return m_bp; }

    inline T* end() { return m_sp; }             // STL-compatible
    inline const T* end() const { return m_sp; } // points past last element

    inline T* base() { return m_bp; }
    inline const T* base() const { return m_bp; }

    inline void jump(T* dst) { m_sp = dst; }
    inline void jump(size_t dst) { m_sp = m_bp + dst; }

  private:
    T* const m_bp;
    T* m_sp;
};

} // namespace via
