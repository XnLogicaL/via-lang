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
#include "debug.h"
#include "support/utility.h"

namespace via {

struct Nullopt
{};

CONSTANT Nullopt nullopt;

template <typename T>
class Option final
{
  public:
    union Storage {
        T val;
        Nullopt null;

        Storage()
            : null()
        {}
        Storage(T&& val)
            : val(move(val))
        {}
        Storage(const Storage&) {}
        Storage(Storage&&) {}
        ~Storage() {}
    };

  public:
    constexpr Option(Nullopt) noexcept
        : m_has_value(false),
          m_storage()
    {}
    constexpr Option(T val) noexcept
        : m_has_value(true),
          m_storage(forward<T>(val))
    {}

    constexpr ~Option() noexcept
    {
        if (has_value()) {
            m_storage.val.~T();
        }
    }

    constexpr operator bool() const noexcept { return has_value(); }
    constexpr T& operator*() noexcept { return get_value(); }
    constexpr T* operator->() noexcept { return &get_value(); }
    constexpr const T& operator*() const noexcept { return get_value(); }
    constexpr const T* operator->() const noexcept { return &get_value(); }

  public:
    [[nodiscard]] constexpr bool has_value() const noexcept { return m_has_value; }

    [[nodiscard]] constexpr T& get_value() noexcept { return m_storage.val; }
    [[nodiscard]] constexpr const T& get_value() const noexcept
    {
        debug::require(has_value(), "Bad Option<T> access");
        return m_storage.val;
    }

    [[nodiscard]] constexpr T&& take_value() noexcept
    {
        debug::require(has_value(), "Bad Option<T> access");
        m_has_value = false;
        return std::move(m_storage.val);
    }

    [[nodiscard]] constexpr T value_or(T orelse) const noexcept
        requires(std::is_copy_constructible_v<T>)
    {
        return has_value() ? get_value() : orelse;
    }

  private:
    Storage m_storage;
    bool m_has_value;
};

} // namespace via
