// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_DEBUG_H_
#define VIA_CORE_DEBUG_H_

#include <via/config.h>
#include <via/types.h>

#undef assert

namespace via {

void assert(bool cond, String message);
void bug(String what);
void todo(String what);
void unimplemented(String what);

}  // namespace via

#endif
