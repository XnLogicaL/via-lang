/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <concepts>
#include <type_traits>
#include <variant>
#include <via/config.hpp>

#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
    #include <cstdlib>
    #include <cxxabi.h>
    #include <memory>
    #include <string>
#endif

namespace via {

template <typename Derived, typename... Bases>
concept derived_from = (std::derived_from<Derived, Bases> || ...);

template <typename T>
concept scoped_enum = std::is_scoped_enum_v<T>;

template <typename T, typename Variant>
struct is_variant_alternative: std::false_type
{};

template <typename T, typename... Ts>
struct is_variant_alternative<T, std::variant<Ts...>>
    : std::bool_constant<(std::is_same_v<T, Ts> || ...)>
{};

template <typename T, typename V>
concept variant_alternative = is_variant_alternative<T, V>::value;

template <typename T>
struct function_traits;

template <typename Ret, typename... Args>
struct function_traits<Ret(Args...)>
{
    static constexpr std::size_t argc = sizeof...(Args);
    using returns = Ret;
    using parameters = std::tuple<Args...>;
};

template <typename Ret, typename... Args>
struct function_traits<Ret (*)(Args...)>: function_traits<Ret(Args...)>
{};

namespace detail {
#if defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
inline std::string __demangle(const char* name)
{
    int status = 0;
    char* demangled = abi::__cxa_demangle(name, nullptr, nullptr, &status);
    std::unique_ptr<char, void (*)(void*)> holder(demangled, std::free);
    return (status == 0 && demangled) ? holder.get() : name;
}

    #define VIA_TYPENAME(EXPR) (::via::detail::__demangle(typeid(EXPR).name()))
#else
    #define VIA_TYPENAME(EXPR) (typeid(EXPR).name())
#endif

} // namespace detail
} // namespace via
