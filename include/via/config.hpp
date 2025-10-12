/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#if defined(_WIN32) || defined(_WIN64)
    #define VIA_PLATFORM_WINDOWS
#elif defined(__linux__)
    #ifdef __ANDROID__
        #define VIA_PLATFORM_ANDROID
    #else
        #define VIA_PLATFORM_LINUX
    #endif
#elif defined(__APPLE__) && defined(__MACH__)
    #include <TargetConditionals.h>
    #if TARGET_OS_IPHONE || TARGET_IPHONE_SIMULATOR
        #define VIA_PLATFORM_IOS
    #else
        #define VIA_PLATFORM_OSX
    #endif
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) ||             \
    defined(__bsdi__) || defined(__DragonFly__)
    #define VIA_PLATFORM_BSD
#elif defined(__EMSCRIPTEN__)
    #define VIA_PLATFORM_EMSCRIPTEN
#else
    #define VIA_PLATFORM_UNKNOWN
#endif

#if defined(VIA_PLATFORM_LINUX) || defined(VIA_PLATFORM_OSX) || defined(VIA_PLATFORM_BSD)
    #define VIA_PLATFORM_POSIX
#endif

#if defined(VIA_PLATFORM_POSIX) || defined(VIA_PLATFORM_ANDROID)
    #define VIA_PLATFORM_UNIX
#endif

#ifdef __GNUC__
    #ifdef __clang__
        #define VIA_COMPILER_CLANG
    #else
        #define VIA_COMPILER_GCC
    #endif
#elif defined(_MSC_VER)
    #define VIA_COMPILER_MSVC
#endif

#ifdef VIA_PLATFORM_CLANG
    #define DISABLE_WARNING_PUSH _Pragma("clang diagnostic push")
    #define DISABLE_WARNING_POP _Pragma("clang diagnostic pop")
    #define DISABLE_WARNING(w) _Pragma(#w)
#elif defined(VIA_PLATFORM_GCC)
    #define DISABLE_WARNING_PUSH _Pragma("GCC diagnostic push")
    #define DISABLE_WARNING_POP _Pragma("GCC diagnostic pop")
    #define DISABLE_WARNING(w) _Pragma(#w)
#elif defined(VIA_PLATFORM_MSVC)
    #define DISABLE_WARNING_PUSH _Pragma("warning(push)")
    #define DISABLE_WARNING_POP _Pragma("warning(pop)")
    #define DISABLE_WARNING(w) _Pragma(#w)
#else
    #define DISABLE_WARNING_PUSH
    #define DISABLE_WARNING_POP
    #define DISABLE_WARNING(w)
#endif

#ifdef VIA_COMPILER_MSVC
    #define VIA_EXPORT __declspec(dllexport)
    #define VIA_NOINLINE __declspec(noinline)
#elif defined(VIA_COMPILER_GCC) || defined(VIA_COMPILER_CLANG)
    #define VIA_EXPORT __attribute__((visibility("default")))
    #define VIA_NOINLINE __attribute__((noinline))
#else
    #define VIA_EXPORT
    #define VIA_NOINLINE
#endif

#define VIA_CONSTANT inline constexpr
