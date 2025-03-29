// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_COMMON_MACROS_H
#define _VIA_COMMON_MACROS_H

#include <iostream>
#include <cstdlib>

#if __has_include(<stacktrace>) && __cplusplus >= 202302L
#include <stacktrace>
#define VIA_HAS_STACKTRACE 1
#else
#define VIA_HAS_STACKTRACE 0
#endif

// Namespace macros
#define VIA_NAMESPACE_BEGIN      namespace via {
#define VIA_NAMESPACE_IMPL_BEGIN namespace via::impl {
#define VIA_NAMESPACE_UTIL_BEGIN namespace via::utils {
#define VIA_NAMESPACE_LIB_BEGIN  namespace via::lib {
#define VIA_NAMESPACE_END        }

#define VIA_VERSION "0.27.2"

// Parameter declaration spec macros
#define VIA_RESTRICT __restrict__

// Function declaration spec macros
#define VIA_NO_DISCARD [[nodiscard]]
#ifdef _MSC_VER // Microsoft Visual C++
#define VIA_NO_RETURN          __declspec(noreturn)
#define VIA_NO_INLINE          __declspec(noinline)
#define VIA_INLINE             __inline
#define VIA_FORCE_INLINE       __forceinline
#define VIA_INLINE_HOT         __forceinline
#define VIA_INLINE_HOT_NODEBUG __forceinline
#define VIA_UNREACHABLE        __assume(0)
#define VIA_FUNCTION_SIGNATURE __FUNCSIG__
#else // GCC / Clang
#define VIA_NO_RETURN          __attribute__((__noreturn__))
#define VIA_NO_INLINE          __attribute__((noinline))
#define VIA_INLINE             inline
#define VIA_FORCE_INLINE       inline __attribute__((always_inline))
#define VIA_INLINE_HOT         inline __attribute__((always_inline, hot))
#define VIA_INLINE_HOT_NODEBUG inline __attribute__((always_inline, hot, __nodebug__))
#define VIA_UNREACHABLE        __builtin_unreachable()
#define VIA_FUNCTION_SIGNATURE __PRETTY_FUNCTION__
#endif

// Branch prediction macros
#ifdef _MSC_VER
#define VIA_LIKELY(a)   (a)
#define VIA_UNLIKELY(a) (a)
#else
#define VIA_LIKELY(a)   (__builtin_expect(!!(a), 1))
#define VIA_UNLIKELY(a) (__builtin_expect(!!(a), 0))
#endif

// Class spec macros
#define VIA_DEFAULT_CONSTRUCTOR(target) target() = default;
#define VIA_DEFAULT_DESTRUCTOR(target)  ~target() = default;
#define VIA_CUSTOM_CONSTRUCTOR(target)  target();
#define VIA_CUSTOM_DESTRUCTOR(target)   ~target();

#define VIA_NON_DEFAULT_CONSTRUCTIBLE(target) target() = delete;
#define VIA_NON_COPYABLE(target)                                                                   \
  target& operator=(const target&) = delete;                                                       \
  target(const target&) = delete;

#define VIA_ALIGN_8 alignas(8)

#if VIA_HAS_STACKTRACE == 1
#define VIA_STACKTRACE std::stacktrace::current()
#else
#define VIA_STACKTRACE ""
#endif

// Utility macros
#define VIA_ASSERT(condition, message)                                                             \
  if (!(condition)) {                                                                              \
    std::cerr << "VIA_ASSERT(): assertion '" << #condition << "' failed.\n"                        \
              << "File: " << __FILE__ << " | Line: " << __LINE__                                   \
              << " | Function: " << VIA_FUNCTION_SIGNATURE << "\nMessage: " << message << "\n";    \
    if (VIA_HAS_STACKTRACE) {                                                                      \
      std::cerr << "Call stack:\n" << VIA_STACKTRACE << '\n';                                      \
    }                                                                                              \
    std::abort();                                                                                  \
  }

// Alias macros
#define u8  uint8_t
#define u16 uint16_t
#define u32 uint32_t
#define u64 uint64_t

#define i8  int8_t
#define i16 int16_t
#define i32 int32_t
#define i64 int64_t

#endif
