// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_LEXLOC_H
#define VIA_HAS_HEADER_LEXLOC_H

#include "common.h"

namespace via {

struct LexLocation {
  size_t line;
  size_t offset;
  size_t begin;
  size_t end;
};

} // namespace via

#endif
