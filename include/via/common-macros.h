// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_common_macros_h
#define vl_has_header_common_macros_h

#include <iostream>
#include <cstdlib>

#if __has_include(<stacktrace>) && __cplusplus >= 202302L
#include <stacktrace>
#define vl_hasstacktrace 1
#else
#define vl_hasstacktrace 0
#endif

#define vl_version "0.27.2"

// Parameter declaration spec macros
#define vl_restrict __restrict__

// Function declaration spec macros
#define vl_nodiscard   [[nodiscard]]
#define vl_noreturn    __attribute__((__noreturn__))
#define vl_noinline    __attribute__((noinline))
#define vl_inline      inline
#define vl_forceinline inline __attribute__((always_inline))
#define vl_optimize    inline __attribute__((always_inline, hot))
#define vl_unreachable __builtin_unreachable()
#define vl_funcsig     __PRETTY_FUNCTION__

// Branch prediction macros
#define vl_likely(a)   (__builtin_expect(!!(a), 1))
#define vl_unlikely(a) (__builtin_expect(!!(a), 0))

// Class spec macros
#define vl_defconstructor(target) target() = default;
#define vl_defdestructor(target)  ~target() = default;

#define vl_nocopy(target)                                                                          \
  target& operator=(const target&) = delete;                                                       \
  target(const target&) = delete;

#define vl_align(x) alignas(x)

#if VIA_HAS_STACKTRACE == 1
#define vl_stacktrace std::stacktrace::current()
#else
#define vl_stacktrace ""
#endif

// Utility macros
#define vl_assert(condition, message)                                                              \
  if (!(condition)) {                                                                              \
    std::cerr << "vl_assert(): assertion '" << #condition << "' failed.\n"                         \
              << "File: " << __FILE__ << " | Line: " << __LINE__ << " | Function: " << vl_funcsig  \
              << "\nMessage: " << message << "\n";                                                 \
    if (vl_hasstacktrace) {                                                                        \
      std::cerr << "Call stack:\n" << vl_stacktrace << '\n';                                       \
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
