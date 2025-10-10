/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.hpp>
#include "traits.hpp"

namespace via {
inline namespace literals {

template <scoped_enum Enum>
constexpr auto operator~(Enum a) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return ~static_cast<T>(a);
}

template <scoped_enum Enum>
constexpr auto operator&(Enum a, Enum b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<T>(a) & static_cast<T>(b);
}

template <scoped_enum Enum>
constexpr auto operator&(std::underlying_type_t<Enum> a, Enum b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return a & static_cast<T>(b);
}

template <scoped_enum Enum>
constexpr auto operator&(Enum a, std::underlying_type_t<Enum> b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<T>(a) & b;
}

template <scoped_enum Enum>
constexpr Enum& operator&=(Enum& a, Enum b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    a = static_cast<Enum>(static_cast<T>(a) & static_cast<T>(b));
    return a;
}

template <scoped_enum Enum>
constexpr Enum& operator&=(Enum& a, std::underlying_type_t<Enum> b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    a = static_cast<Enum>(static_cast<T>(a) & b);
    return a;
}

template <scoped_enum Enum>
constexpr auto operator|(Enum a, Enum b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<T>(a) | static_cast<T>(b);
}

template <scoped_enum Enum>
constexpr auto operator|(std::underlying_type_t<Enum> a, Enum b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return a | static_cast<T>(b);
}

template <scoped_enum Enum>
constexpr auto operator|(Enum a, std::underlying_type_t<Enum> b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<T>(a) | b;
}

template <scoped_enum Enum>
constexpr Enum& operator|=(Enum& a, Enum b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    a = static_cast<Enum>(static_cast<T>(a) | static_cast<T>(b));
    return a;
}

template <scoped_enum Enum>
constexpr Enum& operator|=(Enum& a, std::underlying_type_t<Enum> b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    a = static_cast<Enum>(static_cast<T>(a) | b);
    return a;
}

template <scoped_enum Enum>
constexpr auto operator^(Enum a, Enum b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<T>(a) ^ static_cast<T>(b);
}

template <scoped_enum Enum>
constexpr auto operator^(std::underlying_type_t<Enum> a, Enum b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return a ^ static_cast<T>(b);
}

template <scoped_enum Enum>
constexpr auto operator^(Enum a, std::underlying_type_t<Enum> b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    return static_cast<T>(a) ^ b;
}

template <scoped_enum Enum>
constexpr Enum& operator^=(Enum& a, Enum b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    a = static_cast<Enum>(static_cast<T>(a) ^ static_cast<T>(b));
    return a;
}

template <scoped_enum Enum>
constexpr Enum& operator^=(Enum& a, std::underlying_type_t<Enum> b) noexcept
{
    using T = std::underlying_type_t<Enum>;
    a = static_cast<Enum>(static_cast<T>(a) ^ b);
    return a;
}

} // namespace literals
} // namespace via
