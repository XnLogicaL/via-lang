// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_OBJECT_H
#define _VIA_OBJECT_H

#include "common-macros.h"
#include "common-defs.h"

VIA_NAMESPACE_BEGIN

#ifdef VIA_64BIT

using TInteger = int64_t;
using TFloat = float64_t;

#else

using TInteger = int32_t;
using TFloat = float32_t;

#endif

struct State;
struct TString;
struct TTable;
struct TObject;
struct TFunction;
struct TCFunction;

enum class ValueType : uint8_t {
  nil,            // Empty type, null
  integer,        // Integer type
  floating_point, // Floating point type
  boolean,        // Boolean type
  string,         // String type, pointer to TString
  function,       // Function type, pointer to TFunction
  cfunction,      // CFunction type, pointer to TCFunction
  table,          // Table type, pointer to TTable
  object,         // Object type, pointer to TObject
};

struct VIA_ALIGN_8 TValue {
  ValueType type;
  union {
    TInteger val_integer;      // Integer value
    TFloat val_floating_point; // Floating point value
    bool val_boolean;          // Boolean value
    void* val_pointer;         // Pointer to complex types (string, function, etc.)
  };

  VIA_CUSTOM_DESTRUCTOR(TValue);
  VIA_NON_COPYABLE(TValue);

  TValue(TValue&& other);
  TValue& operator=(TValue&&);

  explicit TValue()
      : type(ValueType::nil) {}

  explicit TValue(bool b)
      : type(ValueType::boolean),
        val_boolean(b) {}

  explicit TValue(TInteger x)
      : type(ValueType::integer),
        val_integer(x) {}

  explicit TValue(TFloat x)
      : type(ValueType::floating_point),
        val_floating_point(x) {}

  explicit TValue(TString* ptr)
      : type(ValueType::string),
        val_pointer(ptr) {}

  explicit TValue(TTable* ptr)
      : type(ValueType::table),
        val_pointer(ptr) {}

  explicit TValue(TFunction* ptr)
      : type(ValueType::function),
        val_pointer(ptr) {}

  explicit TValue(TCFunction* ptr)
      : type(ValueType::cfunction),
        val_pointer(ptr) {}

  explicit TValue(TObject* ptr)
      : type(ValueType::object),
        val_pointer(ptr) {}

  explicit TValue(ValueType type, void* ptr)
      : type(type),
        val_pointer(ptr) {}

  // Returns whether if the object holds a given type.
  VIA_FORCE_INLINE constexpr bool is(ValueType other) const {
    return type == other;
  }

  VIA_FORCE_INLINE constexpr bool is_nil() const {
    return is(ValueType::nil);
  }

  VIA_FORCE_INLINE constexpr bool is_bool() const {
    return is(ValueType::boolean);
  }

  VIA_FORCE_INLINE constexpr bool is_int() const {
    return is(ValueType::integer);
  }

  VIA_FORCE_INLINE constexpr bool is_float() const {
    return is(ValueType::floating_point);
  }

  VIA_FORCE_INLINE constexpr bool is_number() const {
    return is_int() || is_float();
  }

  VIA_FORCE_INLINE constexpr bool is_string() const {
    return is(ValueType::string);
  }

  VIA_FORCE_INLINE constexpr bool is_table() const {
    return is(ValueType::table);
  }

  VIA_FORCE_INLINE constexpr bool is_subscriptable() const {
    return is_string() || is_table();
  }

  VIA_FORCE_INLINE constexpr bool is_function() const {
    return is(ValueType::function);
  }

  VIA_FORCE_INLINE constexpr bool is_cfunction() const {
    return is(ValueType::cfunction);
  }

  VIA_FORCE_INLINE constexpr bool is_callable() const {
    return is_function() || is_cfunction();
  }

  // Returns a clone of the object.
  TValue clone() const;

  // Frees the internal resources of the object and resets union tag to nil.
  void reset();

  // Compares self with a given TValue.
  [[nodiscard]] bool compare(const TValue& other) const;

  // Moves the value and returns it as an rvalue reference.
  [[nodiscard]] VIA_FORCE_INLINE TValue&& move() {
    return static_cast<TValue&&>(*this);
  }

  [[nodiscard]] VIA_FORCE_INLINE const TValue&& move() const {
    return static_cast<const TValue&&>(*this);
  }

  // Returns the pointer value as a pointer to type T.
  template<typename T>
  [[nodiscard]] VIA_FORCE_INLINE T* cast_ptr() {
    return reinterpret_cast<T*>(val_pointer);
  }

  template<typename T>
  [[nodiscard]] VIA_FORCE_INLINE const T* cast_ptr() const {
    return reinterpret_cast<const T*>(val_pointer);
  }
};

struct TString {
  uint32_t len;
  uint32_t hash;
  char* data;

  VIA_CUSTOM_DESTRUCTOR(TString);

  explicit TString(const TString&);
  explicit TString(State*, const char*);

  size_t size();
  TValue get_string(size_t position);
  void set_string(size_t position, const TValue& value);
};

struct THashNode {
  const char* key;
  TValue value;
  THashNode* next;
};

struct TTable {
  size_t arr_capacity = 64;
  size_t ht_capacity = 1024;
  size_t arr_size_cache = 0;
  size_t ht_size_cache = 0;

  bool arr_size_cache_valid = true;
  bool ht_size_cache_valid = true;

  TValue* arr_array = new TValue[arr_capacity];
  THashNode** ht_buckets = new THashNode*[ht_capacity];

  VIA_DEFAULT_CONSTRUCTOR(TTable);
  VIA_CUSTOM_DESTRUCTOR(TTable);

  TTable(const TTable&);

  // Returns the real size of the table.
  size_t size();

  // Returns the element that lives in the given index.
  // Returns nil upon failure.
  TValue get_table(size_t position);
  TValue get_table(const char* key);

  // Sets the element that lives in the given index to the given value.
  void set_table(size_t position, const TValue& value);
  void set_table(const char* key, const TValue& value);
};

struct TObject {
  size_t field_count;

  TValue constructor;
  TValue destructor;
  TValue operator_overloads[16];

  TValue* fields;

  VIA_DEFAULT_CONSTRUCTOR(TObject);
  VIA_CUSTOM_DESTRUCTOR(TObject);

  TObject(size_t field_count)
      : field_count(field_count),
        fields(new TValue[field_count]) {}
};

VIA_NAMESPACE_END

#endif
