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

#include <concepts>
#include <new>
#include <type_traits>
#include <utility>

namespace via {

struct Nullopt
{};

VIA_CONSTANT Nullopt nullopt;

template <typename T> class Option final
{
    static_assert(
        !std::is_same_v<T, Nullopt>,
        "Option<T> cannot be instantiated with Nullopt type"
    );

  public:
    constexpr Option() noexcept
        : m_has_value(false)
    {}

    constexpr Option(Nullopt) noexcept
        : m_has_value(false)
    {}

    // construct from T
    template <typename U = T>
        requires std::constructible_from<T, U&&>
    constexpr Option(U&& value) noexcept(std::is_nothrow_constructible_v<T, U&&>)
        : m_has_value(true)
    {
        ::new (static_cast<void*>(std::addressof(m_storage))) T(std::forward<U>(value));
    }

    // in-place constructor
    template <typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr Option(std::in_place_t, Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args...>)
        : m_has_value(true)
    {
        ::new (static_cast<void*>(std::addressof(m_storage)))
            T(std::forward<Args>(args)...);
    }

    // copy ctor (only if T is copy-constructible)
    constexpr Option(const Option& other)
        requires std::is_copy_constructible_v<T>
        : m_has_value(false)
    {
        if (other.m_has_value) {
            ::new (static_cast<void*>(std::addressof(m_storage))) T(other.value());
            m_has_value = true;
        }
    }

    // move ctor (only if T is move-constructible)
    constexpr Option(Option&& other) noexcept(std::is_nothrow_move_constructible_v<T>)
        requires std::is_move_constructible_v<T>
        : m_has_value(false)
    {
        if (other.m_has_value) {
            ::new (static_cast<void*>(std::addressof(m_storage)))
                T(std::move(other.value()));
            m_has_value = true;
            other.reset();
        }
    }

    ~Option() noexcept { reset(); }

    // --- assignment ---
    constexpr Option& operator=(Nullopt) noexcept
    {
        reset();
        return *this;
    }

    constexpr Option& operator=(const Option& other)
        requires std::is_copy_constructible_v<T> && std::is_copy_assignable_v<T>
    {
        if (&other == this)
            return *this;
        if (other.m_has_value) {
            if (m_has_value) {
                value() = other.value();
            }
            else {
                ::new (static_cast<void*>(std::addressof(m_storage))) T(other.value());
                m_has_value = true;
            }
        }
        else {
            reset();
        }
        return *this;
    }

    constexpr Option& operator=(Option&& other
    ) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_assignable_v<T>)
        requires std::is_move_constructible_v<T> && std::is_move_assignable_v<T>
    {
        if (&other == this)
            return *this;
        if (other.m_has_value) {
            if (m_has_value) {
                value() = std::move(other.value());
            }
            else {
                ::new (static_cast<void*>(std::addressof(m_storage)))
                    T(std::move(other.value()));
                m_has_value = true;
            }
            other.reset();
        }
        else {
            reset();
        }
        return *this;
    }

    template <typename U = T>
        requires std::constructible_from<T, U&&> && std::is_assignable_v<T&, U&&>
    constexpr Option& operator=(U&& value)
    {
        if (m_has_value) {
            this->value() = std::forward<U>(value);
        }
        else {
            ::new (static_cast<void*>(std::addressof(m_storage)))
                T(std::forward<U>(value));
            m_has_value = true;
        }
        return *this;
    }

    // --- observers / accessors ---
    [[nodiscard]] constexpr bool has_value() const noexcept { return m_has_value; }
    constexpr operator bool() const noexcept { return has_value(); }

    [[nodiscard]] constexpr T& value() &
    {
        debug::require(has_value(), "Bad Option<T> access: no value");
        return *ptr();
    }

    [[nodiscard]] constexpr const T& value() const&
    {
        debug::require(has_value(), "Bad Option<T> access: no value");
        return *ptr();
    }

    [[nodiscard]] constexpr T&& value() &&
    {
        debug::require(has_value(), "Bad Option<T> access: no value");
        return std::move(*ptr());
    }

    // returns pointer or nullptr
    [[nodiscard]] constexpr T* get() noexcept { return m_has_value ? ptr() : nullptr; }
    [[nodiscard]] constexpr const T* get() const noexcept
    {
        return m_has_value ? ptr() : nullptr;
    }

    constexpr T& operator*() & { return value(); }
    constexpr const T& operator*() const& { return value(); }
    constexpr T* operator->() { return get(); }
    constexpr const T* operator->() const { return get(); }

    // value_or: accepts any U convertible to T (copyable fallback)
    template <typename U>
    constexpr T value_or(U&& fallback) const
        requires std::is_copy_constructible_v<T> && std::convertible_to<U&&, T>
    {
        return m_has_value ? value() : static_cast<T>(std::forward<U>(fallback));
    }

    // emplace
    template <typename... Args>
        requires std::constructible_from<T, Args...>
    constexpr T& emplace(Args&&... args)
        noexcept(std::is_nothrow_constructible_v<T, Args...>)
    {
        reset();
        ::new (static_cast<void*>(std::addressof(m_storage)))
            T(std::forward<Args>(args)...);
        m_has_value = true;
        return *ptr();
    }

    // reset/destroy
    constexpr void reset() noexcept
    {
        if (m_has_value) {
            ptr()->~T();
            m_has_value = false;
        }
    }

    // swap
    constexpr void swap(Option& other
    ) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_swappable_v<T>)
    {
        if (m_has_value && other.m_has_value) {
            using std::swap;
            swap(value(), other.value());
        }
        else if (m_has_value) {
            // this has value, other doesn't
            ::new (static_cast<void*>(std::addressof(other.m_storage)))
                T(std::move(value()));
            other.m_has_value = true;
            reset();
        }
        else if (other.m_has_value) {
            ::new (static_cast<void*>(std::addressof(m_storage)))
                T(std::move(other.value()));
            m_has_value = true;
            other.reset();
        }
    }

    // comparisons (== / !=). Note: only compares T when both have value.
    friend constexpr bool operator==(const Option& a, const Option& b)
    {
        if (a.m_has_value != b.m_has_value)
            return false;
        if (!a.m_has_value)
            return true;
        return a.value() == b.value();
    }

    friend constexpr bool operator!=(const Option& a, const Option& b)
    {
        return !(a == b);
    }

    //  conversion to T if user wants to extract
    constexpr operator T() const
        requires std::is_copy_constructible_v<T>
    {
        return value();
    }

  private:
    constexpr T* ptr() noexcept
    {
        return std::launder(reinterpret_cast<T*>(static_cast<void*>(m_storage)));
    }

    constexpr const T* ptr() const noexcept
    {
        return std::launder(reinterpret_cast<const T*>(static_cast<const void*>(m_storage)
        ));
    }

  private:
    // storage: properly aligned raw storage for T
    alignas(T) unsigned char m_storage[sizeof(T)];
    bool m_has_value = false;
};

// non-member helpers
template <typename T, typename... Args>
constexpr Option<T> make_option_inplace(Args&&... args)
{
    return Option<T>(std::in_place, std::forward<Args>(args)...);
}

template <typename T> constexpr Option<std::decay_t<T>> make_option(T&& v)
{
    return Option<std::decay_t<T>>(std::forward<T>(v));
}

// ADL swap
template <typename T>
constexpr void swap(Option<T>& a, Option<T>& b) noexcept(noexcept(a.swap(b)))
{
    a.swap(b);
}

} // namespace via
