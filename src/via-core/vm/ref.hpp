/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <via/config.hpp>
#include "value.hpp"

namespace via {

class VirtualMachine;
class ValueRef final
{
  public:
    friend class VirtualMachine;

  public:
    ValueRef(VirtualMachine* vm)
        : m_ptr(nullptr)
    {}

    ValueRef(VirtualMachine* vm, Value* ptr)
        : m_ptr(ptr)
    {}

    template <typename... Args>
    explicit ValueRef(VirtualMachine* vm, Args&&... args)
        : m_ptr(Value::create(vm, std::forward<Args>(args)...))
    {}

    ValueRef(const ValueRef& other)
        : m_ptr(other.m_ptr)
    {
        if (!other.is_null()) {
            other.m_ptr->m_rc++;
        }
    }

    ValueRef(ValueRef&& other)
        : m_ptr(other.m_ptr)
    {
        other.m_ptr = nullptr;
    }

    ~ValueRef()
    {
        if (!is_null()) {
            free();
        }
    }

    ValueRef& operator=(const ValueRef& other)
    {
        if (this != &other) {
            if (!other.is_null()) {
                other.m_ptr->m_rc++;
            }

            if (!is_null()) {
                free();
            }

            this->m_ptr = other.m_ptr;
        }

        return *this;
    }

    ValueRef& operator=(ValueRef&& other)
    {
        if (this != &other) {
            if (!is_null()) {
                free();
            }

            m_ptr = other.m_ptr;
            other.m_ptr = nullptr;
        }

        return *this;
    }

    Value* operator->() const
    {
        debug::require(!is_null(), "attempt to read NULL reference (operator->)");
        return m_ptr;
    }

    Value& operator*() const
    {
        debug::require(!is_null(), "attempt to read NULL reference (operator*)");
        return *m_ptr;
    }

  public:
    inline Value* get() const { return m_ptr; }
    inline bool is_null() const { return m_ptr == nullptr; }

    inline void free()
    {
        debug::require(!is_null(), "free called on NULL reference");
        m_ptr->unref();
        m_ptr = nullptr;
    }

    inline size_t ref_count() const
    {
        debug::require(!is_null(), "ref_count() called on NULL reference");
        return m_ptr->m_rc;
    }

  private:
    Value* m_ptr;
};

} // namespace via
