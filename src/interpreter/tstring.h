// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_STRING_H
#define VIA_HAS_HEADER_STRING_H

#include "common.h"

#include <utility/ustring.h>

namespace via {

// Constant-sized owning String object. Stores String hash.
struct String {
  char* data;
  size_t data_size;
  uint32_t hash;

  VIA_IMPLCOPY(String);
  VIA_IMPLMOVE(String);

  inline explicit String(const char* str)
    : data(duplicate_string(str)),
      data_size(std::strlen(str)),
      hash(hash_string_custom(str)) {}

  inline ~String() {
    delete[] data;
  }

  // Returns the character that lives in the given position.
  String get(size_t position);

  // Sets the character that lives in the given position to the given character.
  void set(size_t position, const String& value);
};

} // namespace via

#endif
