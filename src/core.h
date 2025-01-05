/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "magic_enum.hpp"
#include "except.h"
#include <cassert>
#include <stdexcept>

#define ASMJIT_STATIC

// clang-format off

// Asserts <cond>
// If false, throws an exception that includes debug information such as file name and line that the macro was initially expanded
#define VIA_ASSERT(cond, msg) \
    { \
        if (!(cond)) \
        { \
            std::string err = std::format("VIA_ASSERT(): {}\n  in {}:{}", (msg), __FILE__, __LINE__); \
            throw via::VRTException(err); \
        } \
    }

// Like VIA_ASSERT but doesn't contain debug information
#define VIA_ASSERT_SILENT(cond, msg) \
    { \
        if (!(cond)) \
            throw via::VRTException(msg); \
    }

#define UNREACHABLE() \
    VIA_ASSERT(false, "Unreachable");

#define ENUM_NAME(expr) magic_enum::enum_name(expr)
#define ENUM_CAST(T, expr) magic_enum::enum_cast<T>(expr)

#define VIA_LIKELY(expr) __builtin_expect(!!(expr), 1)
#define VIA_UNLIKELY(expr) __builtin_expect(!!(expr), 0)

#if defined(__GNUC__) || defined(__clang__)
#   define VIA_RESTRICT __restrict__
#   define VIA_FORCEINLINE inline
#else // In case of MSVC or other compilers (MSVC is fucking weird)
#   define VIA_RESTRICT __restrict
#   define VIA_FORCEINLINE __forceinline
#endif

