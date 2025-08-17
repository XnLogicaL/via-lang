// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_POLICY_H_
#define VIA_POLICY_H_

#include "config.h"

#ifdef VIA_PLATFORM_UNKNOWN
#error Unknown/unsupported platform. List of supported platforms: Windows, Linux, IOS, OSX, BSD, EMSCRIPTEN or any UNIX/POSIX compliant OS.
#endif

#ifdef VIA_COMPILER_UNKNOWN
#error Unknown/unsupported compiler. List of supported compilers: MSVC, GCC, CLANG or any adapters for these compilers like MinGW or Msys.
#endif

#endif
