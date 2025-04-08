//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_COMMON_MACROS_H
#define VIA_HAS_HEADER_COMMON_MACROS_H

#define C_GCC   0 // g++ compiler
#define C_CLANG 1 // clang++ compiler
#define C_MSVC  2 // msvc compiler

#ifdef __GNUC__
#ifdef __clang__
#define VIA_COMPILER C_CLANG
#else
#define VIA_COMPILER C_GCC
#endif
#elif defined(_MSC_VER)
#define VIA_COMPILER C_MSVC
#else
#error "Unknown compiler"
#endif

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmultistatement-macros"
#endif

// Check if libstacktrace is available.
#if __has_include(<stacktrace>) && __cplusplus >= 202302L
#include <stacktrace>
#define VIA_HASSTACKTRACE 1
#else
#define VIA_HASSTACKTRACE 0
#endif

// Version information. Should match with git commit version.
#define VIA_VERSION "0.28.7"

#if VIA_COMPILER == C_MSVC
#define VIA_RESTRICT       __restrict
#define VIA_NOMANGLE       extern "C"
#define VIA_NODISCARD      [[nodiscard]]
#define VIA_NORETURN       __declspec(noreturn)
#define VIA_NOINLINE       __declspec(noinline)
#define VIA_INLINE         inline
#define VIA_FORCEINLINE    __forceinline
#define VIA_OPTIMIZE       __forceinline // MSVC doesn't have 'hot' attribute
#define VIA_IMPLEMENTATION inline
#define VIA_UNREACHABLE()  __assume(0)
#define VIA_FUNCSIG        __FUNCSIG__
#define VIA_LIKELY(x)      (x) // No branch prediction hints in MSVC
#define VIA_UNLIKELY(x)    (x)
#else // GCC / Clang
#define VIA_RESTRICT       __restrict__
#define VIA_NOMANGLE       extern "C"
#define VIA_NODISCARD      [[nodiscard]]
#define VIA_NORETURN       __attribute__((noreturn))
#define VIA_NOINLINE       __attribute__((noinline))
#define VIA_INLINE         inline
#define VIA_FORCEINLINE    inline __attribute__((always_inline))
#define VIA_OPTIMIZE       inline __attribute__((always_inline, hot))
#define VIA_IMPLEMENTATION inline
#define VIA_UNREACHABLE()  __builtin_unreachable()
#define VIA_FUNCSIG        __PRETTY_FUNCTION__
#define VIA_LIKELY(a)      (__builtin_expect(!!(a), 1))
#define VIA_UNLIKELY(a)    (__builtin_expect(!!(a), 0))
#endif

/**
 * Makes the target class or struct uncopyable in terms of copy semantics.
 * Must be used inside class or struct clause.
 */
#define VIA_NOCOPY(target)                                                                         \
  target& operator=(const target&) = delete;                                                       \
  target(const target&) = delete;

/**
 * Makes the target class implement custom copy semantics.
 * Must be used inside class or struct clause.
 */
#define VIA_IMPLCOPY(target)                                                                       \
  target& operator=(const target&);                                                                \
  target(const target&);

/**
 * Makes the target class or struct unmovable in terms of move semantics.
 * Must be used inside class or struct clause.
 */
#define VIA_NOMOVE(target)                                                                         \
  target& operator=(target&&) = delete;                                                            \
  target(target&&) = delete;

/**
 * Makes the target class implement custom move semantics.
 * Must be used inside class or struct clause.
 */
#define VIA_IMPLMOVE(target)                                                                       \
  target& operator=(target&&);                                                                     \
  target(target&&);

#if VIA_HASSTACKTRACE == 1
#define VIA_STACKTRACE std::stacktrace::current()
#else
#define VIA_STACKTRACE ""
#endif

// ====================================================================================================
// Utility macros

/**
 * Custom assertion macro that contains debug information like condition, file, line, message and
 * stacktrace on some platforms.
 * Uses stderr to buffer the output and then calls std::abort().
 */
#define VIA_ASSERT(condition, message)                                                             \
  if (!(condition)) {                                                                              \
    std::cerr << "VIA_ASSERT(): assertion '" << #condition << "' failed.\n"                        \
              << "location: " << __FILE__ << ":" << __LINE__ << "\nmessage: " << message << "\n";  \
    if (VIA_HASSTACKTRACE) {                                                                       \
      std::cerr << "callstack:\n" << VIA_STACKTRACE << '\n';                                       \
    }                                                                                              \
    std::abort();                                                                                  \
  }

#if VIA_COMPILER == C_GCC
#pragma GCC diagnostic pop
#endif

#endif
