// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#if not defined(__GNUC__) && not defined(__clang__)
    #pragma error(Unsupported compiler.Please use g++ or clang++.)
#endif

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
#include <limits>
#include <typeindex>
#include <iterator>
// External imports
#include "magic_enum.hpp"
#include "linenoise.hpp"
#include "arena.h"
// Internal imports
#include "token.h"
#include "program.h"

#define ASMJIT_STATIC

// Asserts a condition along with a message.
#define VIA_ASSERT(cond, message) \
    if (!(cond)) { \
        std::cerr << "VIA_ASSERT(): assertion '" << #cond << "' failed.\n" \
                  << " file " << __FILE__ << " line " << __LINE__ << "\n message: " << message \
                  << "\n" \
                  << " callstack:\n" \
                  << std::stacktrace::current() << '\n'; \
        std::abort(); \
    }

// TODO: Make sure this is accurate
#define VIA_VERSION "0.19"

#define VIA_RESTRICT __restrict__
#define VIA_NORETURN __attribute__((__noreturn__))
#define VIA_NOINLINE __attribute__((noinline))
#define VIA_INLINE inline
#define VIA_FORCEINLINE inline __attribute__((always_inline))
#define VIA_MAXOPTIMIZE inline __attribute__((always_inline, hot))
#define VIA_UNREACHABLE __builtin_unreachable();
#define VIA_LIKELY(expr) (__builtin_expect(!!(expr), 1))
#define VIA_UNLIKELY(expr) (__builtin_expect(!!(expr), 0))

namespace via {

// Forward declarations
struct TValue;
struct TTable;
struct TString;
struct TFunction;
struct TCFunction;

// Thanks Terry A. Davis (R.I.P. king) for the idea
using U8  = std::uint8_t;
using U16 = std::uint16_t;
using U32 = std::uint32_t;
using U64 = std::uint64_t;

using I8  = std::int8_t;
using I16 = std::int16_t;
using I32 = std::int32_t;
using I64 = std::int64_t;

using SIZE = std::size_t;

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

VIA_INLINE std::string memdump(const void *ptr, U64 size)
{
    std::ostringstream oss;
    const uint8_t     *bytePtr = reinterpret_cast<const uint8_t *>(ptr);

    oss << "Memory dump at: " << ptr << " (size: " << size << " bytes)\n";

    for (size_t i = 0; i < size; i += 16) {
        oss << std::setw(6) << std::setfill('0') << std::hex << i << " | ";

        for (size_t j = 0; j < 16 && i + j < size; ++j) {
            oss << std::setw(2) << std::setfill('0') << std::hex << static_cast<int>(bytePtr[i + j])
                << " ";
        }

        oss << " | ";

        for (size_t j = 0; j < 16 && i + j < size; ++j) {
            char c = bytePtr[i + j];
            oss << (c >= 32 && c <= 126 ? c : '.');
        }

        oss << '\n';
    }
    oss << std::dec; // Reset formatting

    return oss.str();
}

VIA_INLINE void dump_memory(const void *ptr, U64 size)
{
    std::cout << memdump(ptr, size);
}

} // namespace via
