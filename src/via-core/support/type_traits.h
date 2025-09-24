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
#include <via/config.h>

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
concept default_constructible = std::is_default_constructible_v<T>;

template <typename T>
concept trivially_default_constructible = std::is_trivially_default_constructible_v<T>;

template <typename T>
concept scoped_enum = std::is_scoped_enum_v<T>;

template <typename T, template <typename> typename... Concepts>
concept all = (Concepts<T>::value && ...);

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
