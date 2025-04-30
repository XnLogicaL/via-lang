// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

/**
 * @file object.h
 * @brief Declares the core `Value` type, a tagged union for the runtime.
 *
 * This is a polymorphic container for all dynamically typed runtime values.
 * It efficiently stores and handles different value types including numbers,
 * booleans, strings, arrays, dictionaries, and closures.
 */
#ifndef VIA_HAS_HEADER_OBJECT_H
#define VIA_HAS_HEADER_OBJECT_H

#include "common.h"
#include <magic_enum/magic_enum.hpp>

// MSVC is annoying with uninitialized members
#if VIA_COMPILER == C_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

/**
 * @namespace via
 * @ingroup via_namespace
 * @{
 */
namespace via {

// Forward declarations for pointer types stored in Value
struct State;
struct String;
struct Array;
struct Dict;
struct Closure;

/**
 * @struct Value
 * @brief Polymorphic tagged union representing any runtime value in via.
 *
 * This type is used throughout the via VM to hold and manipulate values
 * of different types dynamically at runtime.
 */
struct alignas(8) Value {
  /**
   * @enum Tag
   * @brief Discriminates the active member of the Value union.
   */
  enum class Tag : uint8_t {
    Nil,      ///< Null or "empty" value.
    Int,      ///< Integer value.
    Float,    ///< Floating-point value.
    Bool,     ///< Boolean value.
    String,   ///< Pointer to `String`.
    Function, ///< Pointer to `Closure`.
    Array,    ///< Pointer to `Array`.
    Dict      ///< Pointer to `Dict`.
  } type;

  /**
   * @union Un
   * @brief Holds the actual value for the current tag.
   */
  union Un {
    int i;         ///< Integer.
    float f;       ///< Float.
    bool b;        ///< Boolean.
    String* str;   ///< Heap string pointer.
    Array* arr;    ///< Heap array pointer.
    Dict* dict;    ///< Heap dictionary pointer.
    Closure* clsr; ///< Function closure pointer.
  } u;

  VIA_NOCOPY(Value);
  VIA_IMPLMOVE(Value);

  ~Value();

  // Constructors
  explicit Value();             ///< Default constructor (Nil).
  explicit Value(bool b);       ///< Constructs a Bool.
  explicit Value(int x);        ///< Constructs an Int.
  explicit Value(float x);      ///< Constructs a Float.
  explicit Value(String* ptr);  ///< Constructs a String.
  explicit Value(Array* ptr);   ///< Constructs an Array.
  explicit Value(Dict* ptr);    ///< Constructs a Dict.
  explicit Value(Closure* ptr); ///< Constructs a Closure.

  Value clone() const; ///< Deep copy of the value.
  void reset();        ///< Clears the value and resets to Nil.

  // clang-format off
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is(Tag other) const { return type == other; }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_nil() const { return is(Tag::Nil); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_bool() const { return is(Tag::Bool); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_int() const { return is(Tag::Int); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_float() const { return is(Tag::Float); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_number() const { return is_int() || is_float(); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_string() const { return is(Tag::String); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_array() const { return is(Tag::Array); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_dict() const { return is(Tag::Dict); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_subscriptable() const { return is_string() || is_array() || is_dict(); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_function() const { return is(Tag::Function); }
  // clang-format on

  // Conversions
  VIA_NODISCARD Value to_integer() const;                  ///< Attempts to convert to Int.
  VIA_NODISCARD Value to_float() const;                    ///< Attempts to convert to Float.
  VIA_NODISCARD Value to_boolean() const;                  ///< Converts to Bool (truthiness).
  VIA_NODISCARD Value to_string() const;                   ///< Converts to a String object.
  VIA_NODISCARD std::string to_cxx_string() const;         ///< Converts to a std::string.
  VIA_NODISCARD std::string to_literal_cxx_string() const; ///< String with literals escaped.

  // Type representation
  VIA_NODISCARD Value type_string() const;           ///< Returns type name as via::String.
  VIA_NODISCARD std::string type_cxx_string() const; ///< Returns type name as std::string.

  /**
   * @brief Attempts to obtain a raw pointer for the value.
   *
   * Only heap-allocated types (e.g., strings, arrays, dicts, closures) will
   * return a valid pointer. Used primarily for hashing and identity checks.
   *
   * @return Pointer or nullptr.
   */
  VIA_NODISCARD void* to_pointer() const;

  // Length functions
  VIA_NODISCARD Value length() const;      ///< Returns the "length" of value if possible.
  VIA_NODISCARD size_t cxx_length() const; ///< Returns native length.

  // Comparison
  VIA_NODISCARD bool compare(const Value& other) const; ///< Deep equality check.
};

} // namespace via

/** @} */

#if VIA_COMPILER == C_MSVC
#pragma warning(pop)
#endif

#endif // VIA_HAS_HEADER_OBJECT_H
