// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file tarray.h
 * @brief Declares the Array structure used for dynamic, indexed storage in the virtual machine.
 *
 * The Array structure implements a dynamic array of `Value`s with automatic resizing
 * and index-based access. Arrays are core collection types in the via language runtime.
 */
#ifndef VIA_HAS_HEADER_ARRAY_H
#define VIA_HAS_HEADER_ARRAY_H

#include "common.h"
#include "csize.h"
#include "tvalue.h"

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

/**
 * @brief Default starting capacity for all arrays.
 */
inline constexpr size_t ARRAY_INITAL_CAPACITY = 64;

/**
 * @struct Array
 * @brief A growable, dynamically sized array of `Value` elements.
 * @see CSize
 *
 * This structure wraps a heap-allocated buffer of `Value` entries and supports
 * index-based access with automatic capacity expansion. Internally, resizing is
 * delegated to the `CSize` helper, which tracks the logical size and performs bounds checks.
 */
struct Array {
  Value* data = nullptr;                        ///< Pointer to array data buffer.
  size_t data_capacity = ARRAY_INITAL_CAPACITY; ///< Allocated capacity.
  CSize csize;                                  ///< Logical size and resizing helper.

  VIA_IMPLCOPY(Array);
  VIA_IMPLMOVE(Array);

  Array();
  ~Array();

  /**
   * @brief Returns the number of initialized elements in the array.
   * @return Current logical size of the array.
   */
  size_t size() const;

  /**
   * @brief Returns the value at the given index.
   * If out of bounds, returns a reference to Nil.
   * @param position Index to fetch.
   * @return Reference to the value at the given index, or Nil.
   */
  Value& get(size_t position);

  /**
   * @brief Assigns a value to the element at the given index.
   * If the index exceeds the current size, the array is resized.
   * @param position Index to assign to.
   * @param value The value to assign.
   */
  void set(size_t position, Value&& value);
};

} // namespace via

/** @} */

#endif
