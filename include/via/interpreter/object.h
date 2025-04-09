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
struct IString;
struct IArray;
struct IDict;
struct IObject;
struct IFunction;

// C function pointer type alias.
using cfunction_t = void (*)(state*);

// Value object type.
enum class IValueType : uint8_t {
  nil,            // Empty type, null
  integer,        // Integer type
  floating_point, // Floating point type
  boolean,        // Boolean type
  string,         // String type, pointer to IString
  function,       // IFunction type, pointer to IFunction
  cfunction,      // CFunction type, function pointer
  array,
  dict,
  object, // Object type, pointer to IObject
};

// Optimized tagged union that acts as a "value object".
struct alignas(8) IValue {
  IValueType type;
  union {
    int val_integer;          // Integer value
    float val_floating_point; // Floating point value
    bool val_boolean;         // Boolean value
    IString* val_string;
    IArray* val_array;
    IDict* val_dict;
    IFunction* val_function;
    cfunction_t val_cfunction;
    IObject* val_object;
  };

  //  ==================
  // [ Object semantics ]
  //  ==================

  // Make uncopyable
  VIA_NOCOPY(IValue);
  // Implement custom move semantics
  VIA_IMPLMOVE(IValue);

  // Destructor
  VIA_IMPLEMENTATION ~IValue() {
    reset();
  }

  VIA_IMPLEMENTATION explicit IValue()
    : type(IValueType::nil) {}

  VIA_IMPLEMENTATION explicit IValue(bool b)
    : type(IValueType::boolean),
      val_boolean(b) {}

  VIA_IMPLEMENTATION explicit IValue(int x)
    : type(IValueType::integer),
      val_integer(x) {}

  VIA_IMPLEMENTATION explicit IValue(float x)
    : type(IValueType::floating_point),
      val_floating_point(x) {}

  VIA_IMPLEMENTATION explicit IValue(IString* ptr)
    : type(IValueType::string),
      val_string(ptr) {}

  VIA_IMPLEMENTATION explicit IValue(IArray* ptr)
    : type(IValueType::array),
      val_array(ptr) {}

  VIA_IMPLEMENTATION explicit IValue(IDict* ptr)
    : type(IValueType::dict),
      val_dict(ptr) {}

  VIA_IMPLEMENTATION explicit IValue(IFunction* ptr)
    : type(IValueType::function),
      val_function(ptr) {}

  VIA_IMPLEMENTATION explicit IValue(cfunction_t ptr)
    : type(IValueType::cfunction),
      val_cfunction(ptr) {}

  VIA_IMPLEMENTATION explicit IValue(IObject* ptr)
    : type(IValueType::object),
      val_object(ptr) {}

  explicit IValue(const char* str);

  //  ==============
  // [ Core methods ]
  //  ==============

  // Returns a deep clone of the object.
  VIA_NODISCARD IValue clone() const;

  // Frees the internal resources of the object and resets union tag to nil.
  void reset();

  //  ===============
  // [ Query methods ]
  //  ===============

  // clang-format off
  // Returns whether if the object holds a given type.
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is(IValueType other) const { return type == other; }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_nil() const { return is(IValueType::nil); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_bool() const { return is(IValueType::boolean); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_int() const { return is(IValueType::integer); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_float() const { return is(IValueType::floating_point); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_number() const { return is_int() || is_float(); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_string() const { return is(IValueType::string); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_array() const { return is(IValueType::array); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_dict() const { return is(IValueType::dict); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_subscriptable() const { return is_string() || is_array() || is_dict(); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_function() const { return is(IValueType::function); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_cfunction() const { return is(IValueType::cfunction); }
  VIA_NODISCARD VIA_FORCEINLINE constexpr bool is_callable() const { return is_function() || is_cfunction(); }
  // clang-format on

  //  ================
  // [ Object methods ]
  //  ================

  // Returns the int interpretation of the value or nil if impossible.
  VIA_NODISCARD IValue to_integer() const;

  // Returns the fp interpretation of the value or nil if impossible.
  VIA_NODISCARD IValue to_float() const;

  // Returns the boolean interpretation of the value.
  VIA_NODISCARD IValue to_boolean() const;

  // Returns the string of the value.
  VIA_NODISCARD IValue to_string() const;
  VIA_NODISCARD std::string to_cxx_string() const;
  VIA_NODISCARD std::string to_literal_cxx_string() const;

  // Returns the type of the value as a string.
  VIA_NODISCARD IValue type_string() const;
  VIA_NODISCARD std::string type_cxx_string() const;

  /**
   * Converts the value into a memory address if possible. Returns nullptr if the object contains a
   * non-heap-allocated type.
   */
  VIA_NODISCARD void* to_pointer() const;

  // Returns the length of the underlying type of the value or nil if impossible.
  VIA_NODISCARD IValue length() const;
  VIA_NODISCARD size_t cxx_length() const;

  // Compares equality between self and a given IValue.
  VIA_NODISCARD bool compare(const IValue& other) const;
};

struct IString {
  size_t len;
  uint32_t hash;
  char* data;

  VIA_IMPLEMENTATION explicit IString(const char* str)
    : len(std::strlen(str)),
      hash(hash_string_custom(str)),
      data(duplicate_string(str)) {}

  VIA_IMPLEMENTATION explicit IString(const IString& other)
    : len(other.len),
      hash(other.hash),
      data(duplicate_string(other.data)) {}

  VIA_IMPLEMENTATION ~IString() {
    delete[] data;
  }

  IValue get(size_t position);
  void set(size_t position, const IValue& value);
};

struct IArray {
  // Capacity of the array. Corresponds to the size of the data array.
  size_t capacity = 64;

  // Size caching
  mutable size_t size_cache = 0;
  mutable bool size_cache_valid = true;

  // Internal data pointer.
  IValue* data = nullptr;

  // Copyable
  VIA_IMPLCOPY(IArray);
  VIA_IMPLMOVE(IArray);

  // Constructor
  VIA_IMPLEMENTATION IArray()
    : data(new IValue[capacity]) {}

  VIA_IMPLEMENTATION ~IArray() {
    delete[] data;
  }

  size_t size() const;

  // Returns the element that lives in the given index or nil.
  IValue& get(size_t position);

  // Sets the element at the given index to the given value. Resizes the array if necessary.
  void set(size_t position, IValue value);
};

struct IHashNode {
  const char* key;
  IValue value;

  inline ~IHashNode() = default;
};

struct IDict {
  // Capacity of the dictionary. Corresponds to the size of the data array.
  size_t capacity = 1024;

  // Size caching
  mutable size_t size_cache = 0;
  mutable bool size_cache_valid = true;

  IHashNode* data = nullptr;

  // Constructor
  VIA_IMPLEMENTATION IDict()
    : data(new IHashNode[capacity]) {}

  // Destructor
  VIA_IMPLEMENTATION ~IDict() {
    delete[] data;
  }

  // Copyable
  VIA_IMPLCOPY(IDict);
  VIA_IMPLMOVE(IDict);

  // Returns the real size of the dictionary.
  size_t size() const;

  // Returns the element that lives in the given index or nil.
  IValue& get(const char* key);

  // Sets the element that lives in the given index to the given value.
  void set(const char* key, IValue value);
};

struct IObject {
  size_t field_count;

  IValue constructor;
  IValue destructor;
  IValue operator_overloads[16];

  IValue* fields;

  inline IObject() = default;
  inline ~IObject();
  inline IObject(size_t field_count)
    : field_count(field_count),
      fields(new IValue[field_count]) {}
};

} // namespace via

#endif
