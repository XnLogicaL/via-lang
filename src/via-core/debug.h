// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_DEBUG_H_
#define VIA_CORE_DEBUG_H_

#include <via/config.h>
#include <via/types.h>
#include <source_location>

namespace via
{

namespace debug
{

void assertm(bool cond,
             String message = "<no-message-specified>",
             std::source_location __loc = std::source_location::current());

[[noreturn]] void bug(
    String what,
    std::source_location __loc = std::source_location::current());

[[noreturn]] void todo(
    String what,
    std::source_location __loc = std::source_location::current());

[[noreturn]] void unimplemented(
    String what = "<no-message-specified>",
    std::source_location __loc = std::source_location::current());

}  // namespace debug

}  // namespace via

#endif
