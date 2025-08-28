// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_REGISTER_H_
#define VIA_CORE_REGISTER_H_

#include <via/config.h>
#include <via/types.h>

namespace via
{

namespace config
{

inline constexpr usize register_count = UINT16_MAX;

}

namespace sema
{

namespace registers
{

void reset();
u16 alloc();
void free(u16 reg);

}  // namespace registers

}  // namespace sema

}  // namespace via

#endif
