// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "api-impl.h"
#include "string-utility.h"
#include "object.h"
#include "function.h"
#include "api-aux.h"

namespace via {

using enum value_type;

// Move-assignment operator, moves values from other object
value_obj& value_obj::operator=(value_obj&& other) {
  if (this != &other) {
    reset();

    // Move the value based on type
    switch (other.type) {
    case integer:
      val_integer = other.val_integer;
      break;
    case floating_point:
      val_floating_point = other.val_floating_point;
      break;
    case boolean:
      val_boolean = other.val_boolean;
      break;
    case string:
    case table:
    case function:
    case cfunction:
      val_pointer = other.val_pointer;
      break;
    default:
      break;
    }

    this->type = other.type;
    other.type = value_type::nil;
  }

  return *this;
}

// Move constructor, transfer ownership based on type
value_obj::value_obj(value_obj&& other) {
  reset();

  switch (other.type) {
  case integer:
    val_integer = other.val_integer;
    break;
  case floating_point:
    val_floating_point = other.val_floating_point;
    break;
  case boolean:
    val_boolean = other.val_boolean;
    break;
  case string:
  case table:
  case function:
  case cfunction:
    val_pointer = other.val_pointer;
    break;
  default:
    break;
  }

  this->type = other.type;
  other.type = nil;
}

// Frees the resources of the value_obj depending on type
value_obj::~value_obj() {
  reset();
}

// Return a clone of the value_obj based on its type
value_obj value_obj::clone() const {
  std::cout << "cloning " << magic_enum::enum_name(type) << "\n";
  switch (type) {
  case integer:
    return value_obj(val_integer);
  case floating_point:
    return value_obj(val_floating_point);
  case boolean:
    return value_obj(val_boolean);
  case string:
    return value_obj(cast_ptr<string_obj>()->data);
  case table:
    return value_obj(table, new table_obj(*cast_ptr<table_obj>()));
  case function:
    return value_obj(function, new function_obj(*cast_ptr<function_obj>()));
  case cfunction:
    return value_obj(cfunction, val_pointer);
  default:
    break;
  }

  return value_obj();
}

void value_obj::reset() {
  if (static_cast<uint8_t>(type) >= static_cast<uint8_t>(value_type::string)) {
    if (!val_pointer) {
      return;
    }

    switch (type) {
    case string:
      delete cast_ptr<string_obj>();
      break;
    case table:
      delete cast_ptr<table_obj>();
      break;
    case function:
      delete cast_ptr<function_obj>();
      break;
    default:
      break;
    }

    val_pointer = nullptr;
    type = nil;
  }
}

bool value_obj::compare(const value_obj&) const {
  return false;
}

value_obj value_obj::to_string() const {
  return impl::__to_string(*this);
}

std::string value_obj::to_cxx_string() const {
  return impl::__to_cxx_string(*this);
}

std::string value_obj::to_literal_cxx_string() const {
  return impl::__to_literal_cxx_string(*this);
}

value_obj::value_obj(const char* str)
  : type(string),
    val_pointer(new string_obj(str)) {}

string_obj::~string_obj() {
  delete[] data;
}

size_t string_obj::size() {
  return len;
}

void string_obj::set(size_t position, const value_obj& value) {
  VIA_ASSERT(position < len, "String index position out of bounds");
  VIA_ASSERT(value.is_string(), "Setting string index to non-string value");

  const string_obj* val = value.cast_ptr<string_obj>();

  VIA_ASSERT(val->len == 1, "Setting string index to non-character string");

  data[position] = value.cast_ptr<string_obj>()->data[0];
}

value_obj string_obj::get(size_t position) {
  VIA_ASSERT(position < len, "String index position out of bounds");
  char chr = data[position];
  string_obj* tstr = new string_obj(&chr);
  return value_obj(tstr);
}

hash_node_obj::~hash_node_obj() = default;

table_obj::~table_obj() {
  delete[] arr_array;
  delete[] ht_buckets;
}

table_obj::table_obj(const table_obj& other)
  : arr_capacity(other.arr_capacity),
    ht_capacity(other.ht_capacity),
    arr_size_cache_valid(other.arr_size_cache_valid),
    ht_size_cache_valid(other.ht_size_cache_valid) {

  if (other.arr_array) {
    arr_array = new value_obj[arr_capacity];
    for (size_t i = 0; i < arr_capacity; ++i) {
      arr_array[i] = other.arr_array[i].clone();
    }
  }
  else {
    arr_array = nullptr;
  }

  if (other.ht_buckets) {
    for (size_t i = 0; i < ht_capacity; ++i) {
      hash_node_obj& src = other.ht_buckets[i];
      hash_node_obj* dst = &ht_buckets[i];
      dst->key = src.key;
      dst->value = src.value.clone();
    }
  }
  else {
    ht_buckets = nullptr;
  }
}

size_t table_obj::size() {
  return impl::__table_size(this);
}

void table_obj::set(const char* key, const value_obj& value) {
  value_obj index(key);
  impl::__table_set(this, index, value);
}

void table_obj::set(size_t position, const value_obj& value) {
  value_obj index(static_cast<TInteger>(position));
  impl::__table_set(this, index, value);
}

value_obj table_obj::get(const char* key) {
  value_obj index(key);
  return impl::__table_get(this, index);
}

value_obj table_obj::get(size_t position) {
  value_obj index(static_cast<TInteger>(position));
  return impl::__table_get(this, index);
}

object_obj::~object_obj() {
  if (fields) {
    delete[] fields;
    fields = nullptr;
  }
}

} // namespace via
