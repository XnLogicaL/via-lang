// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_REGISTER_H_
#define VIA_CORE_REGISTER_H_

#include <via/config.h>
#include <via/types.h>
#include "context.h"

namespace via {

namespace sema {

int alloc_register(SemaContext& ctx);
void free_register(SemaContext& ctx, int reg);

}  // namespace sema

}  // namespace via

#endif
