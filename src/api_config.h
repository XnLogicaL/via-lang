// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_API_CONFIG_H
#define _VIA_API_CONFIG_H

#define VIA_NAMESPACE_BEGIN      namespace via {
#define VIA_NAMESPACE_IMPL_BEGIN namespace via::impl {
#define VIA_NAMESPACE_UTIL_BEGIN namespace via::utils {
#define VIA_NAMESPACE_LIB_BEGIN  namespace via::lib {
#define VIA_NAMESPACE_END        }

#define VIA_VERSION "0.24"

// Parameter declaration spec macros
#define VIA_RESTRICT __restrict__

// Function decleration spec macros
#define VIA_NO_RETURN          __attribute__((__noreturn__))
#define VIA_NO_INLINE          __attribute__((noinline))
#define VIA_INLINE             inline
#define VIA_FORCE_INLINE       VIA_INLINE __attribute__((always_inline))
#define VIA_INLINE_HOT         VIA_INLINE __attribute__((always_inline, hot))
#define VIA_INLINE_HOT_NODEBUG VIA_INLINE __attribute__((always_inline, hot, __nodebug__))
#define VIA_NO_DISCARD         [[nodiscard]]

// Special statement macros
#define VIA_UNREACHABLE __builtin_unreachable();

// Branch prediciton macros
#define VIA_LIKELY(a)   (__builtin_expect(!!(a), 1))
#define VIA_UNLIKELY(a) (__builtin_expect(!!(a), 0))

// Class spec macros
#define VIA_DEFAULT_CONSTRUCTOR(target) target() = default;
#define VIA_DEFAULT_DESTRUCTOR(target)  ~target() = default;
#define VIA_CUSTOM_CONSTRUCTOR(target)  target();
#define VIA_CUSTOM_DESTRUCTOR(target)   ~target();

#define VIA_NON_DEFAULT_CONSTRUCTIBLE(target) target() = delete;
#define VIA_NON_COPYABLE(target)                                                                   \
    target& operator=(const target&) = delete;                                                     \
    target(const target&)            = delete;

#define VIA_ALIGN_8 alignas(8)

// Utility macros
#define VIA_ASSERT(condition, message)                                                             \
    if (!(condition)) {                                                                            \
        std::cerr << "VIA_ASSERT(): assertion '" << #condition << "' failed.\n"                    \
                  << " file " << __FILE__ << " line " << __LINE__ << "\n message: " << message     \
                  << "\n"                                                                          \
                  << " callstack:\n"                                                               \
                  << std::stacktrace::current() << '\n';                                           \
        std::abort();                                                                              \
    }

#endif
