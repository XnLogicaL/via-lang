// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef vl_has_header_object_h
#define vl_has_header_object_h

#include "common-macros.h"
#include "common-defs.h"

namespace via {

#ifdef VIA_64BIT

using TInteger = int64_t;
using TFloat = float64_t;

#else

using TInteger = int32_t;
using TFloat = float32_t;

#endif

struct state;
struct string_obj;
struct table_obj;
struct object_obj;
struct function_obj;

using cfunction_t = void (*)(state*);

enum class value_type : uint8_t {
  nil,            // Empty type, null
  integer,        // Integer type
  floating_point, // Floating point type
  boolean,        // Boolean type
  string,         // String type, pointer to string_obj
  function,       // Function type, pointer to function_obj
  cfunction,      // CFunction type, pointer to cfunction
  table,          // Table type, pointer to table_obj
  object,         // Object type, pointer to object_obj
};

struct vl_align(8) value_obj {
  value_type type;
  union {
    TInteger val_integer;      // Integer value
    TFloat val_floating_point; // Floating point value
    bool val_boolean;          // Boolean value
    void* val_pointer;         // Pointer to complex types (string, function, etc.)
  };

  vl_nocopy(value_obj);

  value_obj(value_obj&& other);
  value_obj& operator=(value_obj&&);
  ~value_obj();

  explicit value_obj()
    : type(value_type::nil) {}

  explicit value_obj(bool b)
    : type(value_type::boolean),
      val_boolean(b) {}

  explicit value_obj(TInteger x)
    : type(value_type::integer),
      val_integer(x) {}

  explicit value_obj(TFloat x)
    : type(value_type::floating_point),
      val_floating_point(x) {}

  explicit value_obj(string_obj* ptr)
    : type(value_type::string),
      val_pointer(ptr) {}

  explicit value_obj(table_obj* ptr)
    : type(value_type::table),
      val_pointer(ptr) {}

  explicit value_obj(function_obj* ptr)
    : type(value_type::function),
      val_pointer(ptr) {}

  explicit value_obj(cfunction_t ptr)
    : type(value_type::cfunction),
      val_pointer(reinterpret_cast<void*>(ptr)) {}

  explicit value_obj(object_obj* ptr)
    : type(value_type::object),
      val_pointer(ptr) {}

  explicit value_obj(value_type type, void* ptr)
    : type(type),
      val_pointer(ptr) {}

  explicit value_obj(const char* str);
  explicit value_obj(state* state, const char* str);

  // Returns whether if the object holds a given type.
  vl_forceinline constexpr bool is(value_type other) const {
    return type == other;
  }

  vl_forceinline constexpr bool is_nil() const {
    return is(value_type::nil);
  }

  vl_forceinline constexpr bool is_bool() const {
    return is(value_type::boolean);
  }

  vl_forceinline constexpr bool is_int() const {
    return is(value_type::integer);
  }

  vl_forceinline constexpr bool is_float() const {
    return is(value_type::floating_point);
  }

  vl_forceinline constexpr bool is_number() const {
    return is_int() || is_float();
  }

  vl_forceinline constexpr bool is_string() const {
    return is(value_type::string);
  }

  vl_forceinline constexpr bool is_table() const {
    return is(value_type::table);
  }

  vl_forceinline constexpr bool is_subscriptable() const {
    return is_string() || is_table();
  }

  vl_forceinline constexpr bool is_function() const {
    return is(value_type::function);
  }

  vl_forceinline constexpr bool is_cfunction() const {
    return is(value_type::cfunction);
  }

  vl_forceinline constexpr bool is_callable() const {
    return is_function() || is_cfunction();
  }

  // Returns a clone of the object.
  value_obj clone() const;

  // Frees the internal resources of the object and resets union tag to nil.
  void reset();

  // Compares self with a given value_obj.
  [[nodiscard]] bool compare(const value_obj& other) const;

  // Moves the value and returns it as an rvalue reference.
  [[nodiscard]] vl_forceinline value_obj&& move() {
    return static_cast<value_obj&&>(*this);
  }

  [[nodiscard]] vl_forceinline const value_obj&& move() const {
    return static_cast<const value_obj&&>(*this);
  }

  // Returns the pointer value as a pointer to type T.
  template<typename T>
  [[nodiscard]] vl_forceinline T* cast_ptr() {
    return reinterpret_cast<T*>(val_pointer);
  }

  template<typename T>
    requires std::is_pointer_v<T>
  [[nodiscard]] vl_forceinline T cast_ptr() {
    return reinterpret_cast<T>(val_pointer);
  }

  template<typename T>
  [[nodiscard]] vl_forceinline const T* cast_ptr() const {
    return reinterpret_cast<const T*>(val_pointer);
  }

  template<typename T>
    requires std::is_pointer_v<T>
  [[nodiscard]] vl_forceinline const T cast_ptr() const {
    return reinterpret_cast<const T>(val_pointer);
  }
};

struct string_obj {
  uint32_t len;
  uint32_t hash;
  char* data;

  explicit string_obj(const string_obj&);
  explicit string_obj(state*, const char*);
  ~string_obj();

  size_t size();
  value_obj get(size_t position);
  void set(size_t position, const value_obj& value);
};

struct hash_node_obj {
  const char* key;
  value_obj value;
  hash_node_obj* next;
};

struct table_obj {
  size_t arr_capacity = 64;
  size_t ht_capacity = 1024;
  size_t arr_size_cache = 0;
  size_t ht_size_cache = 0;

  bool arr_size_cache_valid = true;
  bool ht_size_cache_valid = true;

  value_obj* arr_array = new value_obj[arr_capacity];
  hash_node_obj** ht_buckets = new hash_node_obj*[ht_capacity];

  table_obj() = default;
  table_obj(const table_obj&);
  ~table_obj();

  // Returns the real size of the table.
  size_t size();

  // Returns the element that lives in the given index.
  // Returns nil upon failure.
  value_obj get(size_t position);
  value_obj get(const char* key);

  // Sets the element that lives in the given index to the given value.
  void set(size_t position, const value_obj& value);
  void set(const char* key, const value_obj& value);
};

struct object_obj {
  size_t field_count;

  value_obj constructor;
  value_obj destructor;
  value_obj operator_overloads[16];

  value_obj* fields;

  object_obj() = default;
  ~object_obj();
  object_obj(size_t field_count)
    : field_count(field_count),
      fields(new value_obj[field_count]) {}
};

} // namespace via

#endif
