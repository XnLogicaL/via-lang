// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_ARRAY_H
#define VIA_HAS_HEADER_ARRAY_H

#include "common.h"
#include "csize.h"
#include "tvalue.h"

namespace via {

inline constexpr size_t ARRAY_INITAL_CAPACITY = 64;

struct Array {
  Value* data = nullptr;
  size_t data_capacity = ARRAY_INITAL_CAPACITY;
  CSize csize;

  VIA_IMPLCOPY(Array);
  VIA_IMPLMOVE(Array);

  Array();
  ~Array();

  size_t size() const;

  // Returns the element that lives in the given index or Nil.
  Value& get(size_t position);

  // Sets the element at the given index to the given value. Resizes the array if necessary.
  void set(size_t position, Value value);
};

} // namespace via

#endif
