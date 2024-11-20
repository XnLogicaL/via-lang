/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "magic_enum.hpp"
#include <cassert>

// clang-format off

// Asserts <cond>
// If false, aborts after throwing an error that includes debug information such as file name and line that the macro was initially expanded
#define VIA_ASSERT(cond, err) \
    do \
    { \
        if (!(cond)) \
        { \
            std::cerr << "VIA_ASSERT(): " << (err) << "\n  in file " << __FILE__ << ", line " << __LINE__ << '\n'; \
            std::abort(); \
        } \
    } while (0)

#define ENUM_NAME(expr) magic_enum::enum_name(expr)
#define ENUM_CAST(T, expr) magic_enum::enum_cast<T>(expr)

#define VIA_LIKELY(expr) __builtin_expect(!!(expr), 1)
#define VIA_UNLIKELY(expr) __builtin_expect(!!(expr), 0)

#if defined(__GNUC__) || defined(__clang__)
#   define VIA_RESTRICT __restrict__
#   define VIA_INLINE inline
#else // In case of MSVC or other compilers (MSVC is fucking weird)
#   define VIA_RESTRICT __restrict
#   define VIA_INLINE __forceinline
#endif

