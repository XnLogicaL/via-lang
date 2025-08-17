// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_MODULE_H_
#define VIA_CORE_MODULE_H_

#include <via/config.h>
#include <via/types.h>

namespace via {

namespace config {

namespace module {

inline constexpr auto init_prefix = "viainit_"sv;

}

}  // namespace config

struct ModuleDef;

const ModuleDef* open_module(const char* path, const char* name);

}  // namespace via

#endif
