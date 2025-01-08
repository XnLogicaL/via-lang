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
#include "bytecode.h"
#include "magic_enum.hpp"

// Internal imports
#include "Lexer/token.h"
#include "Parser/ast.h"

#define ASMJIT_STATIC

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

#define ENUM_NAME(expr) magic_enum::enum_name(expr)
#define ENUM_CAST(T, expr) magic_enum::enum_cast<T>(expr)

#if defined(__GNUC__) || defined(__clang__)
    #define VIA_RESTRICT __restrict__
    #define VIA_NORETURN __attribute__((__noreturn__))
    #define VIA_NOINLINE __attribute__((noinline))
    #define VIA_FORCEINLINE inline __attribute__((always_inline))
    #define VIA_UNREACHABLE() __builtin_unreachable()
    #define VIA_LIKELY(expr) __builtin_expect(!!(expr), 1)
    #define VIA_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
#elifdef _MSC_VER // In case of MSVC (MSVC is fucking weird)
    #define VIA_RESTRICT __restrict
    #define VIA_NORETURN __declspec(__noreturn__)
    #define VIA_NOINLINE __declspec(noinline)
    #define VIA_FORCEINLINE __forceinline
    #define VIA_UNREACHABLE() __assume(false)
    #define VIA_LIKELY(expr) expr
    #define VIA_UNLIKELY(expr) expr
#else
    #pragma error(Unsupported compiler detected.Supported compilers are : GNU g++, clang, MSVC(partial))
#endif

namespace via
{

#ifdef VIA_LONGJMP
using JmpOffset = std::int64_t;
#else
using JmpOffset = std::int32_t;
#endif

template<typename K, typename T>
using HashMap = std::unordered_map<K, T>; // General purpose hashmap.
using Hash = std::uint32_t;               // Hash type.
using LabelId = std::string_view;         // Label identifier.
using CallArgc = std::uint8_t;            // Argument count type.
using ExitCode = std::int8_t;             // Exit code type.
using ExitMsg = const char *;             // Exit message type.
using ThreadId = std::uint32_t;           // Thread identifier.
using LocalId = std::uint32_t;            // Local variable identifier (stack offset).
using kGlobId = std::string_view;         // Global constant identifier.
using RegId = uint32_t;                   // Register
using StkPos = std::size_t;               // Stack position.
using StkVal = std::uintptr_t;            // Stack value, castable pointer
using StkAddr = StkVal *;                 // Stack address
using YldTime = float;                    // Yield time type, for the VM.
using TNumber = double;
using TBool = bool;
using TPointer = uintptr_t;
using TableKey = Hash;

// Forward declarations
struct TValue;
struct TTable;
struct TString;
struct TFunction;
struct TCFunction;
using TokenHolder = std::vector<Token>;

VIA_FORCEINLINE char *dupstring(const std::string &str)
{
    size_t len = str.size() + 1;
    char *chars = new char[len];
    std::memcpy(chars, str.c_str(), len);
    return chars;
}

class VRTException : public std::exception
{
public:
    std::string message;

    VRTException(const std::string &message)
        : message(message)
    {
    }

    const char *what() const throw()
    {
        return message.c_str();
    }
};

struct ProgramData
{
    std::string file_name;
    std::string source;
    TokenHolder *tokens;
    AbstractSyntaxTree *ast;
    BytecodeHolder *bytecode;

    ProgramData(std::string file_name, std::string file_source)
        : file_name(file_name)
        , source(file_source)
    {
    }
};

} // namespace via
