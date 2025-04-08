//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================

#ifndef VIA_HAS_HEADER_OBJECT_H
#define VIA_HAS_HEADER_OBJECT_H

#include "common-macros.h"
#include "common-defs.h"
#include "magic_enum/magic_enum.hpp"
#include "string-utility.h"
#include <cstring>

//  ==========
// [ object.h ]
//  ==========
namespace via {

// Forward declarations
struct state;
struct string_obj;
struct array_obj;
struct dict_obj;
struct object_obj;
struct function_obj;

// C function pointer type alias.
using cfunction_t = void (*)(state*);

// Value object type.
enum class value_type : uint8_t {
  nil,            // Empty type, null
  integer,        // Integer type
  floating_point, // Floating point type
  boolean,        // Boolean type
  string,         // String type, pointer to string_obj
  function,       // Function type, pointer to function_obj
  cfunction,      // CFunction type, function pointer
  array,
  dict,
  object, // Object type, pointer to object_obj
};

// Optimized tagged union that acts as a "value object".
struct alignas(8) value_obj {
  value_type type;
  union {
    int val_integer;          // Integer value
    float val_floating_point; // Floating point value
    bool val_boolean;         // Boolean value
    string_obj* val_string;
    array_obj* val_array;
    dict_obj* val_dict;
    function_obj* val_function;
    cfunction_t val_cfunction;
    object_obj* val_object;
  };

  //  ==================
  // [ Object semantics ]
  //  ==================

  // Make uncopyable
  VIA_NOCOPY(value_obj);
  // Implement custom move semantics
  VIA_IMPLMOVE(value_obj);

  // Destructor
  ~value_obj();

  explicit value_obj()
    : type(value_type::nil) {}

  explicit value_obj(bool b)
    : type(value_type::boolean),
      val_boolean(b) {}

  explicit value_obj(int x)
    : type(value_type::integer),
      val_integer(x) {}

  explicit value_obj(float x)
    : type(value_type::floating_point),
      val_floating_point(x) {}

  explicit value_obj(string_obj* ptr)
    : type(value_type::string),
      val_string(ptr) {}

  explicit value_obj(array_obj* ptr)
    : type(value_type::array),
      val_array(ptr) {}

  explicit value_obj(dict_obj* ptr)
    : type(value_type::dict),
      val_dict(ptr) {}

  explicit value_obj(function_obj* ptr)
    : type(value_type::function),
      val_function(ptr) {}

  explicit value_obj(cfunction_t ptr)
    : type(value_type::cfunction),
      val_cfunction(ptr) {}

  explicit value_obj(object_obj* ptr)
    : type(value_type::object),
      val_object(ptr) {}

  explicit value_obj(const char* str);

  //  ==============
  // [ Core methods ]
  //  ==============

  // Returns a deep clone of the object.
  VIA_NODISCARD value_obj clone() const;

  // Frees the internal resources of the object and resets union tag to nil.
  void reset();

  //  ===============
  // [ Query methods ]
  //  ===============

  // clang-format off
  // Returns whether if the object holds a given type.
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is(value_type other) const { return type == other; }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_nil() const { return is(value_type::nil); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_bool() const { return is(value_type::boolean); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_int() const { return is(value_type::integer); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_float() const { return is(value_type::floating_point); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_number() const { return is_int() || is_float(); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_string() const { return is(value_type::string); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_array() const { return is(value_type::array); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_dict() const { return is(value_type::dict); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_subscriptable() const { return is_string() || is_array() || is_dict(); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_function() const { return is(value_type::function); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_cfunction() const { return is(value_type::cfunction); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_callable() const { return is_function() || is_cfunction(); }
  // clang-format on

  //  ================
  // [ Object methods ]
  //  ================

  // Returns the int interpretation of the value or nil if impossible.
  VIA_NODISCARD value_obj to_integer() const;

  // Returns the fp interpretation of the value or nil if impossible.
  VIA_NODISCARD value_obj to_float() const;

  // Returns the boolean interpretation of the value.
  VIA_NODISCARD value_obj to_boolean() const;

  // Returns the string of the value.
  VIA_NODISCARD value_obj to_string() const;
  VIA_NODISCARD std::string to_cxx_string() const;
  VIA_NODISCARD std::string to_literal_cxx_string() const;

  // Returns the type of the value as a string.
  VIA_NODISCARD value_obj type_string() const;
  VIA_NODISCARD std::string type_cxx_string() const;

  /**
   * Converts the value into a memory address if possible. Returns nullptr if the object contains a
   * non-heap-allocated type.
   */
  VIA_NODISCARD void* to_pointer() const;

  // Returns the length of the underlying type of the value or nil if impossible.
  VIA_NODISCARD value_obj length() const;
  VIA_NODISCARD size_t cxx_length() const;

  // Compares equality between self and a given value_obj.
  VIA_NODISCARD bool compare(const value_obj& other) const;

  // Moves the value and returns it as an rvalue reference.
  VIA_NODISCARD VIA_FORCEINLINE value_obj&& move() {
    return static_cast<value_obj&&>(*this);
  }

  // Moves the value and returns it as a constant rvalue reference.
  VIA_NODISCARD VIA_FORCEINLINE const value_obj&& move() const {
    return static_cast<const value_obj&&>(*this);
  }
};

struct string_obj {
  size_t len;
  uint32_t hash;
  char* data;

  VIA_IMPLEMENTATION explicit string_obj(const char* str)
    : len(std::strlen(str)),
      hash(hash_string_custom(str)),
      data(duplicate_string(str)) {}

  VIA_IMPLEMENTATION explicit string_obj(const string_obj& other)
    : len(other.len),
      hash(other.hash),
      data(duplicate_string(other.data)) {}

  VIA_IMPLEMENTATION ~string_obj() {
    delete[] data;
  }

  value_obj get(size_t position);
  void set(size_t position, const value_obj& value);
};

struct array_obj {
  // Capacity of the array. Corresponds to the size of the data array.
  size_t capacity = 64;

  // Size caching
  mutable size_t size_cache = 0;
  mutable bool size_cache_valid = true;

  // Internal data pointer.
  value_obj* data = nullptr;

  // Copyable
  VIA_IMPLCOPY(array_obj);
  VIA_IMPLMOVE(array_obj);

  // Constructor
  VIA_IMPLEMENTATION array_obj()
    : data(new value_obj[capacity]) {}

  VIA_IMPLEMENTATION ~array_obj() {
    delete[] data;
  }

  size_t size() const;

  // Returns the element that lives in the given index or nil.
  value_obj& get(size_t position);

  // Sets the element at the given index to the given value. Resizes the array if necessary.
  void set(size_t position, value_obj value);
};

struct hash_node {
  const char* key;
  value_obj value;

  inline ~hash_node() = default;
};

struct dict_obj {
  // Capacity of the dictionary. Corresponds to the size of the data array.
  size_t capacity = 1024;

  // Size caching
  mutable size_t size_cache = 0;
  mutable bool size_cache_valid = true;

  hash_node* data = nullptr;

  // Constructor
  VIA_IMPLEMENTATION dict_obj()
    : data(new hash_node[capacity]) {}

  // Destructor
  VIA_IMPLEMENTATION ~dict_obj() {
    delete[] data;
  }

  // Copyable
  VIA_IMPLCOPY(dict_obj);
  VIA_IMPLMOVE(dict_obj);

  // Returns the real size of the dictionary.
  size_t size() const;

  // Returns the element that lives in the given index or nil.
  value_obj& get(const char* key);

  // Sets the element that lives in the given index to the given value.
  void set(const char* key, value_obj value);
};

struct object_obj {
  size_t field_count;

  value_obj constructor;
  value_obj destructor;
  value_obj operator_overloads[16];

  value_obj* fields;

  inline object_obj() = default;
  inline ~object_obj();
  inline object_obj(size_t field_count)
    : field_count(field_count),
      fields(new value_obj[field_count]) {}
};

} // namespace via

#endif
