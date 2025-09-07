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
#include <cstring>
#include <type_traits>

namespace via
{

template <typename T>
[[nodiscard]] inline constexpr T&& fwd(
  typename std::remove_reference_t<T>& val) noexcept
{
  return static_cast<T&&>(val);
}

template <typename T>
[[nodiscard]] constexpr typename std::remove_reference_t<T>&& mv(
  T&& val) noexcept
{
  return static_cast<typename std::remove_reference_t<T>&&>(val);
}

template <typename T>
  requires(!std::is_lvalue_reference_v<T>)
[[nodiscard]] inline constexpr T&& fwd(
  typename std::remove_reference_t<T>&& val) noexcept
{
  return static_cast<T&&>(val);
}

template <typename To, typename From>
  requires(sizeof(To) == sizeof(From) && std::is_trivially_copyable_v<To> &&
           std::is_trivially_copyable_v<From>)
[[nodiscard]] inline constexpr To pun_cast(const From& src) noexcept
{
  To dst;
  std::memcpy(&dst, &src, sizeof(To));
  return dst;
}

}  // namespace via
