#pragma once

#include <cassert>
#include <new>
#include <type_traits>
#include <utility>

#include <via/config.h>
#include <via/types.h>
#include "debug.h"
#include "error.h"

namespace via {

template <typename E = ErrorInfo>
class Unexpected final
{
  public:
    Unexpected()
        requires(std::is_default_constructible_v<E>)
    = default;

    template <typename... Args>
        requires(std::is_constructible_v<E, Args...>)
    explicit Unexpected(Args&&... args)
        : m_unexp(E(std::forward<Args>(args)...))
    {}

    // Move out the error value by value
    [[nodiscard]] E take_error() && { return std::move(m_unexp); }

    // If you need an lvalue overload, you must decide semantics.
    // We'll delete it to avoid accidental copies:
    [[nodiscard]] E take_error() & = delete;

  private:
    E m_unexp;
};

template <typename T>
class Expected final
{
    static_assert(
        !std::is_same_v<T, Error>,
        "Expected<T> cannot be instantiated with Error type"
    );

  public:
    Expected() = delete;
    Expected(T val) noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        new (&m_storage.val) T(std::move(val));
        m_has_value = true;
        m_is_valid = true;
    }

    template <typename E>
    Expected(Unexpected<E>&& err) noexcept
    {
        new (&m_storage.err) Error(Error::fail<E>(std::move(err).take_error()));
        debug::require(
            m_storage.err.has_error(),
            "Cannot construct Expected<T> with successful Error"
        );
        m_has_value = false;
        m_is_valid = true;
    }

    NO_COPY(Expected)

    Expected(Expected&& other
    ) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<Error>)
        : m_has_value(false),
          m_is_valid(false)
    {
        if (!other.m_is_valid) {
            // other is already invalid/moved-from: we remain invalid
            return;
        }

        if (other.m_has_value) {
            new (&m_storage.val) T(std::move(other.m_storage.val));
            m_has_value = true;
            m_is_valid = true;
        }
        else {
            new (&m_storage.err) Error(std::move(other.m_storage.err));
            m_has_value = false;
            m_is_valid = true;
        }

        // leave `other` in a safe (non-destroying) state
        other.destroy_active();
    }

    Expected& operator=(Expected&& other
    ) noexcept(std::is_nothrow_move_constructible_v<T> && std::is_nothrow_move_constructible_v<Error>)
    {
        if (this == &other)
            return *this;

        destroy_active();

        if (!other.m_is_valid) {
            // other empty -> remain empty / invalid
            m_is_valid = false;
            m_has_value = false;
            return *this;
        }

        if (other.m_has_value) {
            new (&m_storage.val) T(std::move(other.m_storage.val));
            m_has_value = true;
            m_is_valid = true;
        }
        else {
            new (&m_storage.err) Error(std::move(other.m_storage.err));
            m_has_value = false;
            m_is_valid = true;
        }

        other.destroy_active();
        return *this;
    }

    ~Expected() noexcept { destroy_active(); }

    operator bool() const noexcept { return has_value(); }

    T& operator*() & noexcept { return get_value(); }
    T* operator->() noexcept { return &get_value(); }
    const T& operator*() const& noexcept { return get_value(); }
    const T* operator->() const noexcept { return &get_value(); }

  public:
    [[nodiscard]] bool has_value() const noexcept { return m_is_valid && m_has_value; }
    [[nodiscard]] bool has_error() const noexcept { return m_is_valid && !m_has_value; }

    [[nodiscard]] T& get_value() & noexcept
    {
        debug::require(has_value(), "Bad Expected<T> access (get_value)");
        return m_storage.val;
    }

    [[nodiscard]] const T& get_value() const& noexcept
    {
        debug::require(has_value(), "Bad Expected<T> access (get_value)");
        return m_storage.val;
    }

    [[nodiscard]] Error& get_error() & noexcept
    {
        debug::require(has_error(), "Bad Expected<T> access (get_error)");
        return m_storage.err;
    }

    [[nodiscard]] const Error& get_error() const& noexcept
    {
        debug::require(has_error(), "Bad Expected<T> access (get_error)");
        return m_storage.err;
    }

    [[nodiscard]] T take_value() && noexcept(std::is_nothrow_move_constructible_v<T>)
    {
        debug::require(has_value(), "Bad Expected<T> access (take_value)");
        T tmp = std::move(m_storage.val);
        m_storage.val.~T();
        m_is_valid = false; // prevent destructor from trying to destroy anything
        m_has_value = false;
        return tmp;
    }

    [[nodiscard]] Error
    take_error() && noexcept(std::is_nothrow_move_constructible_v<Error>)
    {
        debug::require(has_error(), "Bad Expected<T> access (take_error)");
        Error tmp = std::move(m_storage.err);
        m_storage.err.~Error();
        m_is_valid = false;
        m_has_value = false;
        return tmp;
    }

    [[nodiscard]] T value_or(const T& orelse) const& noexcept
        requires(std::is_copy_constructible_v<T>)
    {
        return has_value() ? get_value() : orelse;
    }

    [[nodiscard]] Error error_or(const Error& orelse) const& noexcept
    {
        return has_error() ? get_error() : orelse;
    }

  private:
    union Storage {
        Storage() noexcept {}
        ~Storage() noexcept {}

        T val;
        Error err;
    };

    void destroy_active() noexcept
    {
        if (!m_is_valid)
            return;
        if (m_has_value) {
            m_storage.val.~T();
        }
        else {
            m_storage.err.~Error();
        }
        m_is_valid = false;
    }

  private:
    Storage m_storage;
    bool m_has_value = false;
    bool m_is_valid = false;
};

} // namespace via
