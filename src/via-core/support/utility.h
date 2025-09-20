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
#include <via/config.h>
#include <via/types.h>

#define NO_COPY(target)                                                                  \
    target& operator=(const target&) = delete;                                           \
    target(const target&) = delete;

#define IMPL_COPY(target)                                                                \
    target& operator=(const target&);                                                    \
    target(const target&);

#define NO_MOVE(target)                                                                  \
    target& operator=(target&&) = delete;                                                \
    target(target&&) = delete;

#define IMPL_MOVE(target)                                                                \
    target& operator=(target&&);                                                         \
    target(target&&);

// BIT_ENUM macro: defines enum class + bitwise operators
#define BIT_ENUM(name, type)                                                             \
    CONSTANT name operator|(name a, name b)                                              \
    {                                                                                    \
        return static_cast<name>(static_cast<type>(a) | static_cast<type>(b));           \
    }                                                                                    \
    CONSTANT name operator&(name a, name b)                                              \
    {                                                                                    \
        return static_cast<name>(static_cast<type>(a) & static_cast<type>(b));           \
    }                                                                                    \
    CONSTANT name operator^(name a, name b)                                              \
    {                                                                                    \
        return static_cast<name>(static_cast<type>(a) ^ static_cast<type>(b));           \
    }                                                                                    \
    CONSTANT name operator~(name a)                                                      \
    {                                                                                    \
        return static_cast<name>(~static_cast<type>(a));                                 \
    }                                                                                    \
    CONSTANT name& operator|=(name& a, name b)                                           \
    {                                                                                    \
        a = a | b;                                                                       \
        return a;                                                                        \
    }                                                                                    \
    CONSTANT name& operator&=(name& a, name b)                                           \
    {                                                                                    \
        a = a & b;                                                                       \
        return a;                                                                        \
    }                                                                                    \
    CONSTANT name& operator^=(name& a, name b)                                           \
    {                                                                                    \
        a = a ^ b;                                                                       \
        return a;                                                                        \
    }                                                                                    \
    CONSTANT type to_uint(name a)                                                        \
    {                                                                                    \
        return static_cast<type>(a);                                                     \
    }

#define BIT_ENUM_CLASS(name, type)                                                       \
    friend constexpr name operator|(name a, name b)                                      \
    {                                                                                    \
        return static_cast<name>(static_cast<type>(a) | static_cast<type>(b));           \
    }                                                                                    \
    friend constexpr name operator&(name a, name b)                                      \
    {                                                                                    \
        return static_cast<name>(static_cast<type>(a) & static_cast<type>(b));           \
    }                                                                                    \
    friend constexpr name operator^(name a, name b)                                      \
    {                                                                                    \
        return static_cast<name>(static_cast<type>(a) ^ static_cast<type>(b));           \
    }                                                                                    \
    friend constexpr name operator~(name a)                                              \
    {                                                                                    \
        return static_cast<name>(~static_cast<type>(a));                                 \
    }                                                                                    \
    friend constexpr name& operator|=(name& a, name b)                                   \
    {                                                                                    \
        a = a | b;                                                                       \
        return a;                                                                        \
    }                                                                                    \
    friend constexpr name& operator&=(name& a, name b)                                   \
    {                                                                                    \
        a = a & b;                                                                       \
        return a;                                                                        \
    }                                                                                    \
    friend constexpr name& operator^=(name& a, name b)                                   \
    {                                                                                    \
        a = a ^ b;                                                                       \
        return a;                                                                        \
    }                                                                                    \
    friend constexpr type to_uint(name a)                                                \
    {                                                                                    \
        return static_cast<type>(a);                                                     \
    }

namespace via {

using std::forward;
using std::move;

} // namespace via
