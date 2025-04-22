// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_OBJECT_H
#define VIA_HAS_HEADER_OBJECT_H

#include "common.h"
#include "tstring.h"

#include <magic_enum/magic_enum.hpp>

// MSVC is annoying with uninitialized members
#if VIA_COMPILER == C_MSVC
#pragma warning(push)
#pragma warning(disable : 26495)
#endif

//  ==========
// [ object.h ]
//  ==========
namespace via {

// Forward declarations
struct State;
struct Array;
struct Dict;
struct Closure;

// C function pointer type alias.
using cfunction_t = void (*)(State*);

// Optimized tagged union value object.
struct alignas(8) Value {
  const bool owns = true; // Does the value own the pointer in it?
  enum class Tag : uint8_t {
    Nil,      // Empty type, null
    Int,      // Integer type
    Float,    // Floating point type
    Bool,     // Boolean type
    String,   // String type
    Function, // Function type
    Array,
    Dict,
  } type;

  union Un {
    int i;   // Integer value
    float f; // Floating point value
    bool b;  // Boolean value
    String* str;
    Array* arr;
    Dict* dict;
    Closure* clsr;
  } u;

  VIA_NOCOPY(Value);
  VIA_IMPLMOVE(Value);

  inline ~Value() {
    reset();
  }

  explicit Value(bool owns, Tag ty, Un un);
  explicit Value(bool owns = true);
  explicit Value(bool b, bool owns = true);
  explicit Value(int x, bool owns = true);
  explicit Value(float x, bool owns = true);
  explicit Value(String* ptr, bool owns = true);
  explicit Value(Array* ptr, bool owns = true);
  explicit Value(Dict* ptr, bool owns = true);
  explicit Value(Closure* ptr, bool owns = true);
  explicit Value(Tag t, Un u, bool owns = true);
  explicit Value(const char* str);

  // Returns a deep clone of the object.
  VIA_NODISCARD Value clone() const;
  // Returns a clone reference object with the same internal pointer but no ownership.
  VIA_NODISCARD Value weak_clone() const;

  // Frees the internal resources of the object and resets union tag to Nil.
  void reset();

  // clang-format off
  // Returns whether if the object holds a given type.
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

  // Returns the int interpretation of the value or Nil if impossible.
  VIA_NODISCARD Value to_integer() const;

  // Returns the fp interpretation of the value or Nil if impossible.
  VIA_NODISCARD Value to_float() const;

  // Returns the Bool interpretation of the value.
  VIA_NODISCARD Value to_boolean() const;

  // Returns the String of the value.
  VIA_NODISCARD Value to_string() const;
  VIA_NODISCARD std::string to_cxx_string() const;
  VIA_NODISCARD std::string to_literal_cxx_string() const;

  // Returns the type of the value as a String.
  VIA_NODISCARD Value type_string() const;
  VIA_NODISCARD std::string type_cxx_string() const;

  /**
   * Converts the value into a memory address if possible. Returns nullptr if the object contains a
   * non-heap-allocated type. Used mainly for hashing purposes.
   */
  VIA_NODISCARD void* to_pointer() const;

  // Returns the length of the underlying type of the value or Nil if impossible.
  VIA_NODISCARD Value length() const;
  VIA_NODISCARD size_t cxx_length() const;

  // Compares equality between self and a given Value.
  VIA_NODISCARD bool compare(const Value& other) const;

  // Returns whether if the given value is a reference to this.
  VIA_NODISCARD bool is_reference(const Value& other) const;
};

} // namespace via

#if VIA_COMPILER == C_MSVC
#pragma warning(pop)
#endif

#endif
