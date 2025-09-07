// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_UTILITY_H_
#define VIA_CORE_UTILITY_H_

#include <via/config.h>
#include <via/types.h>
#include <type_traits>
#include <utility>
#include "debug.h"

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
[[nodiscard]] inline constexpr bool isa(const From& val)
{
  return To::classof(&val);
}

template <typename To, typename From>
[[nodiscard]] inline constexpr To* cast(From* val)
{
  debug::assertm(isa<To>(val) && "Invalid cast!");
  return static_cast<To*>(val);
}

template <typename To, typename From>
[[nodiscard]] constexpr To* dyn_cast(From* val)
{
  return isa<To>(val) ? static_cast<To*>(val) : nullptr;
}

template <typename To, typename From>
  requires(std::is_default_constructible_v<To> && sizeof(To) == sizeof(From))
[[nodiscard]] inline constexpr To pun_cast(const From& src) noexcept
{
  To dst;
  std::memcpy(&dst, &src, sizeof(To));
  return dst;
}

}  // namespace via

#endif
