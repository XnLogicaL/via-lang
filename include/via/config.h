// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CONFIG_H_
#define VIA_CONFIG_H_

#if defined(_WIN32) || defined(_WIN64)
#define VIA_PLATFORM_WINDOWS
#elifdef __linux__
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
#elif defined(__FreeBSD__) || defined(__NetBSD__) || defined(__OpenBSD__) || \
    defined(__bsdi__) || defined(__DragonFly__)
#define VIA_PLATFORM_BSD
#elifdef __EMSCRIPTEN__
#define VIA_PLATFORM_EMSCRIPTEN
#else
#define VIA_PLATFORM_UNKNOWN
#endif

#if defined(VIA_PLATFORM_LINUX) || defined(VIA_PLATFORM_OSX) || \
    defined(VIA_PLATFORM_BSD)
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
#elifdef _MSC_VER
#define VIA_COMPILER_MSVC
#endif

#ifdef VIA_PLATFORM_WINDOWS
#define VIA_EXPORT __declspec(dllexport)
#else
#define VIA_EXPORT
#endif

#define VIA_NOCOPY(target)                   \
  target& operator=(const target&) = delete; \
  target(const target&) = delete;

#define VIA_IMPLCOPY(target)        \
  target& operator=(const target&); \
  target(const target&);

#define VIA_IMPLCOPY_CONSTEXPR(target)        \
  constexpr target& operator=(const target&); \
  constexpr target(const target&);

#define VIA_NOMOVE(target)              \
  target& operator=(target&&) = delete; \
  target(target&&) = delete;

#define VIA_IMPLMOVE(target)   \
  target& operator=(target&&); \
  target(target&&);

#include "types.h"

namespace via {

namespace config {

inline constexpr usize version = 10;

}

}  // namespace via

#endif
