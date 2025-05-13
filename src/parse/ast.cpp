// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ast.h"

#include <codegen/visitor.h>
#include <codegen/types.h>
#include <interpreter/tvalue.h>

#define depth_tab_space std::string(depth, ' ')

namespace via {} // namespace via
