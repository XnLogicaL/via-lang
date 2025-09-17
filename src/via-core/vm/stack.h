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

CONSTANT usize STACK_SIZE = 8192;

}
} // namespace config

template <typename T>
class Stack final
{
  public:
    Stack(Allocator& pAlloc) :
        m_alloc(pAlloc),
        m_base_ptr(m_alloc.alloc<T>(config::vm::STACK_SIZE)),
        m_stack_ptr(m_base_ptr)
    {}

  public:
    Allocator& get_allocator() { return m_alloc; }

    inline usize size() const { return m_stack_ptr - m_base_ptr; }
    inline void jump(T* dst) { m_stack_ptr = dst; }
    inline void jump(usize dst) { m_stack_ptr = m_base_ptr + dst; }
    inline void push(T val) { *(m_stack_ptr++) = val; }
    inline T pop() { return *(--m_stack_ptr); }
    inline T* top() { return m_stack_ptr - 1; }
    inline T* at(usize idx) { return m_base_ptr + idx; }
    inline T* begin() { return m_base_ptr; }
    inline T* end() { return m_stack_ptr - 1; }

  private:
    Allocator& m_alloc;
    T* const m_base_ptr;
    T* m_stack_ptr;
};

} // namespace via
