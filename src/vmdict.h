// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_TDICT_H
#define VIA_TDICT_H

#include "common.h"
#include "heapbuf.h"
#include "vmval.h"

namespace via {

namespace vm {

struct DictHashNode {
  const char* key;
  Value value;
};

struct Dict {
  DictHashNode* nodes;
};

} // namespace vm

} // namespace via

#endif
