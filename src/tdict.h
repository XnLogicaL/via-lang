// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file tdict.h
 * @brief Declares the Dict structure used for key-value mapping in the via VM.
 *
 * The Dict structure implements a hash table mapping string keys to `Value`s.
 * It is used to represent dictionaries and object-like structures in the via language.
 */

#ifndef VIA_HAS_HEADER_TDICT_H
#define VIA_HAS_HEADER_TDICT_H

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
 * @brief Default starting capacity for all dictionaries.
 */
inline constexpr size_t DICT_INITIAL_CAPACITY = 64;

/**
 * @struct Dict
 * @brief A dynamically allocated hash table mapping `const char*` keys to `Value` objects.
 *
 * This dictionary implementation is based on open addressing (linear probing).
 * Keys are raw C strings assumed to be interned or otherwise stable.
 */
struct Dict {
  /**
   * @struct HNode
   * @brief A single key-value entry within the dictionary hash table.
   */
  struct HNode {
    const char* key; ///< Null-terminated string key (not owned).
    Value value;     ///< Corresponding value.
  }* data = nullptr; ///< Pointer to the hash table buffer.

  size_t data_capacity = DICT_INITIAL_CAPACITY; ///< Total capacity of the table.
  CSize csize;                                  ///< Tracks logical size and handles resizing.

  VIA_IMPLCOPY(Dict); ///< Enables copy constructor and assignment.
  VIA_IMPLMOVE(Dict); ///< Enables move constructor and assignment.

  Dict();
  ~Dict();

  /**
   * @brief Returns the number of key-value pairs currently stored.
   * @return Logical size of the dictionary.
   */
  size_t size() const;

  /**
   * @brief Retrieves the value associated with a given key.
   * If the key is not found, returns a reference to Nil.
   * @param key The null-terminated key string.
   * @return Reference to the value associated with the key or Nil.
   */
  Value& get(const char* key);

  /**
   * @brief Sets the value associated with a given key.
   * If the key already exists, the value is overwritten.
   * If the key does not exist, a new entry is inserted.
   * @param key The null-terminated key string.
   * @param value The value to associate with the key.
   */
  void set(const char* key, Value value);
};

} // namespace via

/** @} */

#endif
