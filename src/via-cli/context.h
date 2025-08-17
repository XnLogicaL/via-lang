// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CLI_CONTEXT_H_
#define VIA_CLI_CONTEXT_H_

#include <via/types.h>

namespace via {

namespace cli {

struct Context {
  String path, emit;
};

}  // namespace cli

}  // namespace via

#endif
