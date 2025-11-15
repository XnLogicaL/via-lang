/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <type_traits>
#include <via/config.hpp>
#include "support/traits.hpp"
#include "support/utility.hpp"

#define DEFINE_ERROR_ENUM(NAME, TYPE) enum class NAME : TYPE

namespace via {

template <scoped_enum E>
    requires requires(E e) {
        { to_string(e) } -> std::convertible_to<std::string_view>;
    }
class Error final
{
  public:
    using Raw = std::underlying_type_t<E>;

  public:
    constexpr Error(E code)
        : m_code(code)
    {}

    constexpr operator E() const { return m_code; }
    constexpr operator Raw() const { return static_cast<Raw>(m_code); }
    constexpr operator std::string_view() const { return to_string(m_code); }

  public:
    constexpr auto code() const { return m_code; }
    constexpr auto raw() const { return static_cast<Raw>(m_code); }
    constexpr auto string() const { return to_string(m_code); }

  private:
    const E m_code;
};

} // namespace via
