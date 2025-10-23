/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <array>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <via/config.hpp>

namespace via {

template <std::integral T = int>
constexpr T iota() noexcept
{
    static T data{0};
    return data++;
}

template <std::integral T = int>
constexpr T ipow(T base, T exp)
{
    T result = 1;
    for (;;) {
        if (exp & 1)
            result *= base;
        exp >>= 1;
        if (!exp)
            break;
        base *= base;
    }

    return result;
}

constexpr size_t hash_combine(size_t seed, size_t v) noexcept
{
    return seed ^ (v + 0x9e3779b97f4a7c15ULL + (seed << 6) + (seed >> 2));
}

constexpr size_t hash_ptr(const void* ptr) noexcept
{
    return reinterpret_cast<size_t>(reinterpret_cast<uintptr_t>(ptr));
}

template <typename... T>
constexpr size_t hash_all(T... args) noexcept
{
    size_t seed = 0;
    ((seed = hash_combine(seed, args)), ...);
    return seed;
}

template <class It, class ElemHash>
constexpr size_t hash_range(It first, It last, ElemHash hash)
{
    size_t seed = hash_combine(0, (size_t)std::distance(first, last));
    for (auto it = first; it != last; ++it)
        seed = hash_combine(seed, hash(*it));
    return seed;
}

} // namespace via
