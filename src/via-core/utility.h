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
#include <string>
#include <type_traits>
#include <typeinfo>

#define NO_COPY(target)                      \
  target& operator=(const target&) = delete; \
  target(const target&) = delete;

#define IMPL_COPY(target)           \
  target& operator=(const target&); \
  target(const target&);

#define NO_MOVE(target)                 \
  target& operator=(target&&) = delete; \
  target(target&&) = delete;

#define IMPL_MOVE(target)      \
  target& operator=(target&&); \
  target(target&&);

// BIT_ENUM macro: defines enum class + bitwise operators
#define BIT_ENUM(name, type)                                               \
  inline constexpr name operator|(name a, name b)                          \
  {                                                                        \
    return static_cast<name>(static_cast<type>(a) | static_cast<type>(b)); \
  }                                                                        \
  inline constexpr name operator&(name a, name b)                          \
  {                                                                        \
    return static_cast<name>(static_cast<type>(a) & static_cast<type>(b)); \
  }                                                                        \
  inline constexpr name operator^(name a, name b)                          \
  {                                                                        \
    return static_cast<name>(static_cast<type>(a) ^ static_cast<type>(b)); \
  }                                                                        \
  inline constexpr name operator~(name a)                                  \
  {                                                                        \
    return static_cast<name>(~static_cast<type>(a));                       \
  }                                                                        \
  inline constexpr name& operator|=(name& a, name b)                       \
  {                                                                        \
    a = a | b;                                                             \
    return a;                                                              \
  }                                                                        \
  inline constexpr name& operator&=(name& a, name b)                       \
  {                                                                        \
    a = a & b;                                                             \
    return a;                                                              \
  }                                                                        \
  inline constexpr name& operator^=(name& a, name b)                       \
  {                                                                        \
    a = a ^ b;                                                             \
    return a;                                                              \
  }                                                                        \
  inline constexpr type to_uint(name a)                                    \
  {                                                                        \
    return static_cast<type>(a);                                           \
  }

#define BIT_ENUM_CLASS(name, type)                                         \
  friend constexpr name operator|(name a, name b)                          \
  {                                                                        \
    return static_cast<name>(static_cast<type>(a) | static_cast<type>(b)); \
  }                                                                        \
  friend constexpr name operator&(name a, name b)                          \
  {                                                                        \
    return static_cast<name>(static_cast<type>(a) & static_cast<type>(b)); \
  }                                                                        \
  friend constexpr name operator^(name a, name b)                          \
  {                                                                        \
    return static_cast<name>(static_cast<type>(a) ^ static_cast<type>(b)); \
  }                                                                        \
  friend constexpr name operator~(name a)                                  \
  {                                                                        \
    return static_cast<name>(~static_cast<type>(a));                       \
  }                                                                        \
  friend constexpr name& operator|=(name& a, name b)                       \
  {                                                                        \
    a = a | b;                                                             \
    return a;                                                              \
  }                                                                        \
  friend constexpr name& operator&=(name& a, name b)                       \
  {                                                                        \
    a = a & b;                                                             \
    return a;                                                              \
  }                                                                        \
  friend constexpr name& operator^=(name& a, name b)                       \
  {                                                                        \
    a = a ^ b;                                                             \
    return a;                                                              \
  }                                                                        \
  friend constexpr type to_uint(name a)                                    \
  {                                                                        \
    return static_cast<type>(a);                                           \
  }

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
  #include <cxxabi.h>
  #include <cstdlib>
  #include <memory>
#endif

namespace via
{

template <std::integral T = int>
constexpr T iota() noexcept
{
  static T data(0);
  return data++;
}

template <typename T>
[[nodiscard]] inline constexpr T&& fwd(std::remove_reference_t<T>& val) noexcept
{
  return static_cast<T&&>(val);
}

template <typename T>
[[nodiscard]] constexpr std::remove_reference_t<T>&& mv(T&& val) noexcept
{
  return static_cast<std::remove_reference_t<T>&&>(val);
}

template <typename T>
  requires(!std::is_lvalue_reference_v<T>)
[[nodiscard]] inline constexpr T&& fwd(
  std::remove_reference_t<T>&& val) noexcept
{
  return static_cast<std::remove_reference_t<T>&&>(val);
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

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
inline std::string demangle(const char* name)
{
  int status = 0;
  char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
  std::unique_ptr<char, void (*)(void*)> holder(demangled, std::free);
  return (status == 0 && demangled) ? holder.get() : name;
}

  #define TYPENAME(expr) (demangle(typeid(expr).name()))
#else
  #define TYPENAME(expr) (typeid(expr).name())
#endif

}  // namespace via
