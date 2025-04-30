// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file tstring.h
 * @brief Defines the `String` type used within the virtual machine and runtime.
 *
 * This is a constant-sized, heap-allocated, hash-cached string structure
 * designed for performance in dictionary lookups and language runtime internals.
 */
#ifndef VIA_HAS_HEADER_STRING_H
#define VIA_HAS_HEADER_STRING_H

#include "common.h"
#include <utility/ustring.h>

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @struct String
 * @brief Constant-sized owning string type used in the via runtime.
 *
 * This structure owns its character data, tracks its size, and caches a hash
 * value to accelerate dictionary operations and comparisons.
 */
struct String {
  char* data;       ///< Heap-allocated UTF-8 character data.
  size_t data_size; ///< Number of bytes in the string (not null-terminated).
  uint32_t hash;    ///< Cached hash for fast comparisons and dict lookup.

  VIA_IMPLCOPY(String);
  VIA_IMPLMOVE(String);

  /**
   * @brief Constructs a new `String` from a null-terminated C-string.
   * @param str The input C-string to copy.
   */
  String(const char* str);
  ~String();

  /**
   * @brief Gets a single-character string at the specified position.
   * @param position Index of the character to retrieve.
   * @return A new `String` containing just that character.
   */
  String get(size_t position);

  /**
   * @brief Replaces the character at the given index with the first character of another string.
   * @param position Index of the character to replace.
   * @param value A string whose first character will be used as the replacement.
   */
  void set(size_t position, const String& value);
};

} // namespace via

/** @} */

#endif
