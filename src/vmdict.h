// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_TDICT_H
#define VIA_TDICT_H

#include "common.h"
#include "heapbuf.h"
#include "vmval.h"

namespace via {

namespace vm {

struct HashNode {
  const char* key;
  Value value;
};

struct Dict {
  HashNode** nodes;
  size_t capacity;
};

size_t dict_size(State* S, Dict* D);
void dict_set(State* S, Dict* D, const char* key, Value&& val);
Value* dict_get(State* S, Dict* D, const char* key);

} // namespace vm

} // namespace via

#endif
