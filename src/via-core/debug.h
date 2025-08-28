// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_DEBUG_H_
#define VIA_CORE_DEBUG_H_

#include <via/config.h>
#include <via/types.h>

namespace via
{

namespace debug
{

void _assert(bool cond, String message);
[[noreturn]] void bug(String what);
[[noreturn]] void todo(String what);
[[noreturn]] void unimplemented(String what = "<no-message-specified>");

}  // namespace debug

}  // namespace via

#endif
