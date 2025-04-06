// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef VIA_HAS_HEADER_COMMON_MACROS_H
#define VIA_HAS_HEADER_COMMON_MACROS_H

#include <iostream>
#include <cstdlib>

#define VIA_USING_GCC   !__clang__&& __GNUC__
#define VIA_USING_CLANG __clang__&& __GNUC__

#if VIA_USING_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultistatement-macros"
#endif

#if __has_include(<stacktrace>) && __cplusplus >= 202302L
#include <stacktrace>
#define VIA_HASSTACKTRACE 1
#else
#define VIA_HASSTACKTRACE 0
#endif

#define VIA_VERSION "0.28.2"

// Parameter declaration spec macros
#define VIA_RESTRICT __restrict__

// Function declaration spec macros
#define VIA_NOMANGLE       extern "C"
#define VIA_NODISCARD      [[nodiscard]]
#define VIA_NORETURN       __attribute__((__noreturn__))
#define VIA_NOINLINE       __attribute__((noinline))
#define VIA_INLINE         inline
#define VIA_FORCEINLINE    inline __attribute__((always_inline))
#define VIA_OPTIMIZE       inline __attribute__((always_inline, hot))
#define VIA_IMPLEMENTATION inline // Used for header-only implementations

#define VIA_UNREACHABLE __builtin_unreachable()
#define VIA_FUNCSIG     __PRETTY_FUNCTION__

// Branch prediction macros
#define VIA_LIKELY(a)   (__builtin_expect(!!(a), 1))
#define VIA_UNLIKELY(a) (__builtin_expect(!!(a), 0))

// Class spec macros
#define VIA_DEFCONSTRUCTOR(target) target() = default;
#define VIA_DEFDESTRUCTOR(target)  ~target() = default;

#define VIA_NOCOPY(target)                                                                         \
  target& operator=(const target&) = delete;                                                       \
  target(const target&) = delete;

#define VIA_IMPLCOPY(target)                                                                       \
  target& operator=(const target&);                                                                \
  target(const target&);

#define VIA_NOMOVE(target)                                                                         \
  target& operator=(const target&&) = delete;                                                      \
  target(const target&&) = delete;

#define VIA_IMPLMOVE(target)                                                                       \
  target& operator=(const target&&);                                                               \
  target(const target&&);

#if VIA_HASSTACKTRACE == 1
#define VIA_STACKTRACE std::stacktrace::current()
#else
#define VIA_STACKTRACE ""
#endif

// Utility macros
#define VIA_ASSERT(condition, message)                                                             \
  if (!(condition)) {                                                                              \
    std::cerr << "VIA_ASSERT(): assertion '" << #condition << "' failed.\n"                        \
              << "location: " << __FILE__ << ":" << __LINE__ << "\nmessage: " << message << "\n";  \
    if (VIA_HASSTACKTRACE) {                                                                       \
      std::cerr << "callstack:\n" << VIA_STACKTRACE << '\n';                                       \
    }                                                                                              \
    std::abort();                                                                                  \
  }

#if VIA_USING_GCC
#pragma GCC diagnostic pop
#endif

#endif
