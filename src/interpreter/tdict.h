// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_TDICT_H
#define VIA_HAS_HEADER_TDICT_H

#include "common.h"
#include "csize.h"
#include "tvalue.h"

namespace via {

inline constexpr size_t DICT_INITIAL_CAPACITY = 64;

struct Dict {
  struct HNode {
    const char* key;
    Value value;
  }* data = nullptr;
  size_t data_capacity = DICT_INITIAL_CAPACITY;
  CSize csize;

  Dict();
  ~Dict();

  VIA_IMPLCOPY(Dict);
  VIA_IMPLMOVE(Dict);

  size_t size() const;

  // Returns the element that lives in the given index or Nil.
  Value& get(const char* key);

  // Sets the element that lives in the given index to the given value.
  void set(const char* key, Value value);
};

} // namespace via

#endif
