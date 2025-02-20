// =========================================================================================== |
// This file is a part of The via Programming Language; see LICENSE for licensing information. |
// =========================================================================================== |

#pragma once

// C++ std imports
#include <bitset>
#include <cassert>
#include <cctype>
#include <cstdbool>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <format>
#include <fstream>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <optional>
#include <stack>
#include <stdexcept>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <variant>
#include <vector>
#include <stacktrace>
#include <shared_mutex>
// External imports
#include "magic_enum.hpp"
#include "linenoise.hpp"
#include "arena.h"
// Internal imports
#include "token.h"

#define ASMJIT_STATIC

// Asserts a condition along with a message.
#define VIA_ASSERT(cond, message) \
    if (!(cond)) { \
        std::cerr << "VIA_ASSERT(): assertion '" << #cond << "' failed.\n" \
                  << " message: " << message << "\n" \
                  << " callstack:\n" \
                  << std::stacktrace::current() << '\n'; \
        std::abort(); \
    }

// TODO: Make sure this is accurate
#define VIA_VERSION "0.16"

#if defined(__GNUC__) || defined(__clang__)
    #define VIA_RESTRICT __restrict__
    #define VIA_NORETURN __attribute__((__noreturn__))
    #define VIA_NOINLINE __attribute__((noinline))
    #define VIA_INLINE inline
    #define VIA_FORCEINLINE inline __attribute__((always_inline))
    #define VIA_MAXOPTIMIZE inline __attribute__((always_inline, hot))
    #define VIA_UNREACHABLE() __builtin_unreachable();
    #define VIA_LIKELY(expr) (__builtin_expect(!!(expr), 1))
    #define VIA_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))
#elifdef _MSC_VER // In case of MSVC (MSVC is fucking weird, please do NOT \
                  // use it)
    #define VIA_RESTRICT __restrict
    #define VIA_NORETURN __declspec(__noreturn__)
    #define VIA_NOINLINE __declspec(noinline)
    #define VIA_INLINE
    #define VIA_FORCEINLINE __forceinline
    #define VIA_MAXOPTIMIZE __forceinline
    #define VIA_UNREACHABLE() __assume(false)
    #define VIA_LIKELY(expr) expr
    #define VIA_UNLIKELY(expr) expr
#else
// clang-format off
    #pragma error(Unsupported compiler detected, supported compilers include: GNU g++, clang, MSVC (partial))
// clang-format on
#endif

namespace via {

// Forward declarations
struct TValue;
struct TTable;
struct TString;
struct TFunction;
struct TCFunction;
struct TokenHolder;
struct AbstractSyntaxTree;
struct BytecodeHolder;

// Thanks Terry A. Davis (R.I.P. king) for the idea
using U8 = std::uint8_t;
using U16 = std::uint16_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;

using I8 = std::int8_t;
using I16 = std::int16_t;
using I32 = std::int32_t;
using I64 = std::int64_t;

#ifdef VIA_LONGJUMP
using JmpOffset = U64;
#else
using JmpOffset = U32;
#endif

VIA_FORCEINLINE char *dup_string(const char *str)
{
    char *chars = new char[std::strlen(str) + 1];
    std::strcpy(chars, str);
    return chars;
}

VIA_FORCEINLINE char *dup_string(const std::string &str)
{
    char *chars = new char[str.size() + 1];
    std::strcpy(chars, str.c_str());
    return chars;
}

template<typename T, typename F>
    requires(std::invocable<F> && std::is_same_v<std::invoke_result_t<F>, T>)
VIA_FORCEINLINE T safe_call(F func, T default_value)
{
    try {
        return func();
    }
    catch (std::exception &) {
        return default_value;
    }
}

struct ProgramData {
    std::string file;
    std::string source;
    TokenHolder *tokens = nullptr;
    AbstractSyntaxTree *ast = nullptr;
    BytecodeHolder *bytecode = nullptr;
    std::vector<TValue> *constants;
    std::map<size_t, std::string> bytecode_info;

    ProgramData(std::string, std::string);
    ~ProgramData();
};

} // namespace via
