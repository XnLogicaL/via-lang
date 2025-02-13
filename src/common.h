/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

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
#include <utility>
#include <variant>
#include <vector>
// External imports
#include "magic_enum.hpp"
#include "arena.hpp"
#include "linenoise.hpp"
// Internal imports
#include "token.h"

#define ASMJIT_STATIC

// Asserts <cond>.
// If false, raises a fatal error that immediately terminates the program.
#define VIA_ASSERT(cond, msg) assert(cond &&msg);

#define ENUM_NAME(expr) magic_enum::enum_name(expr)
#define ENUM_CAST(T, expr) magic_enum::enum_cast<T>(expr)

// TODO: Make sure this is accurate
#define VIA_VERSION ("0.4")

#if defined(__GNUC__) || defined(__clang__)
    #define VIA_RESTRICT __restrict__
    #define VIA_NORETURN __attribute__((__noreturn__))
    #define VIA_NOINLINE __attribute__((noinline))
    #define VIA_INLINE inline
    #define VIA_FORCEINLINE inline __attribute__((always_inline))
    #define VIA_MAXOPTIMIZE inline __attribute__((always_inline, hot))
    #define VIA_UNREACHABLE() __builtin_unreachable()
    #define VIA_LIKELY(expr) __builtin_expect(!!(expr), 1)
    #define VIA_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#elifdef _MSC_VER // In case of MSVC (MSVC is fucking weird, please do NOT use it)
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

namespace via
{

#ifdef VIA_LONGJUMP
using JmpOffset = std::int64_t;
#else
using JmpOffset = std::int32_t;
#endif

// Forward declarations
struct TValue;
struct TTable;
struct TString;
struct TFunction;
struct TCFunction;
struct TokenHolder;
struct AbstractSyntaxTree;
struct BytecodeHolder;

using Hash = std::uint32_t;       // Hash type.
using LabelId = std::string_view; // Label identifier.
using ThreadId = std::uint32_t;   // Thread identifier.
using LocalId = std::uint32_t;    // Local variable identifier (stack offset).
using kGlobId = std::string_view; // Global constant identifier.
using RegId = std::uint32_t;      // Register
using StkPos = std::size_t;       // Stack position.
using YldTime = float;            // Yield time type, for the VM.
using TNumber = double;
using TBool = bool;
using TableKey = Hash;

VIA_FORCEINLINE char *dupstring(const std::string &str)
{
    size_t len = str.size() + 1;
    char *chars = new char[len];
    std::memcpy(chars, str.c_str(), len);
    return chars;
}

template<typename T, typename F>
    requires std::invocable<F> && std::is_same_v<std::invoke_result_t<F>, T>
VIA_FORCEINLINE T safe_call(F func, T default_value)
{
    try
    {
        return func();
    }
    catch (std::exception &)
    {
        return default_value;
    }
}

VIA_FORCEINLINE void print_memory(void *ptr, size_t size)
{
    unsigned char *byte_ptr = static_cast<unsigned char *>(ptr);

    // Print each byte in the memory block
    for (size_t i = 0; i < size; ++i)
    {
        // Print the byte in hexadecimal
        std::cout << std::hex << std::setw(2) << std::setfill('0') << static_cast<int>(byte_ptr[i]) << " ";

        // Print a newline every 16 bytes for readability
        if ((i + 1) % 16 == 0)
        {
            std::cout << std::endl;
        }
    }

    // Print a newline at the end
    std::cout << std::dec << std::endl; // Reset to decimal
}

struct ProgramData
{
    std::string file_name;
    std::string source;
    TokenHolder *tokens;
    AbstractSyntaxTree *ast;
    BytecodeHolder *bytecode;
    std::map<size_t, std::string> bytecode_info;

    ProgramData(std::string, std::string);
    ~ProgramData();
};

} // namespace via
