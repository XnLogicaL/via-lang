/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "core.h"
#include "shared.h"
#include "state.h"

#include "execlinux.h"
#include "execwin.h"

// Check if viaJIT is compatible with the current CPU architecture
#if !defined(__aarch64__) && !defined(__x86_64__) && !defined(__i386__)
#    warning(1:arch_check) viaJIT is currently only available on architectures: x86-64, x86-32 and arm64!
#    define VIA_JIT_ARCH_SUPPORTED (false)
#else
#    define VIA_JIT_ARCH_SUPPORTED (true)
#endif

// Check if viaJIT is compatible with the current OS
#if !defined(__linux__) && !defined(_WIN32) && !defined(_WIN64)
#    warning(2:os_check) viaJIT is currently only available on operating systems Windows and Linux!
#    define VIA_JIT_OS_SUPPORTED (false)
#else
#    define VIA_JIT_OS_SUPPORTED (true)
#endif

// Define shortcut for quickly finding out if viaJIT is supported
#define VIA_JIT_SUPPORTED (VIA_JIT_ARCH_SUPPORTED && VIA_JIT_OS_SUPPORTED)

namespace via::jit
{

void viaJIT_dispatchjit(viaState *);

}