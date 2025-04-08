//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "api-impl.h"
#include "string-utility.h"
#include "object.h"
#include "function.h"
#include "api-aux.h"

namespace via {

using enum value_type;

//  ==============
// [ Value object ]
//  ==============

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
      val_string = other.val_string;
      break;
    case array:
      val_array = other.val_array;
      break;
    case dict:
      val_dict = other.val_dict;
      break;
    case function:
      val_function = other.val_function;
      break;
    case cfunction:
      val_cfunction = other.val_cfunction;
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
    val_string = other.val_string;
    break;
  case array:
    val_array = other.val_array;
    break;
  case dict:
    val_dict = other.val_dict;
    break;
  case function:
    val_function = other.val_function;
    break;
  case cfunction:
    val_cfunction = other.val_cfunction;
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
  switch (type) {
  case integer:
    return value_obj(val_integer);
  case floating_point:
    return value_obj(val_floating_point);
  case boolean:
    return value_obj(val_boolean);
  case string:
    return value_obj(val_string->data);
  case array:
    return value_obj(new array_obj(*val_array));
  case dict:
    return value_obj(new dict_obj(*val_dict));
  case function:
    return value_obj(new function_obj(*val_function));
  case cfunction:
    return value_obj(val_cfunction);
  default:
    break;
  }

  return value_obj();
}

void value_obj::reset() {
  if (static_cast<uint8_t>(type) >= static_cast<uint8_t>(value_type::string)) {
    switch (type) {
    case string:
      delete val_string;
      val_string = nullptr;
      break;
    case array:
      delete val_array;
      val_array = nullptr;
      break;
    case dict:
      delete val_dict;
      val_dict = nullptr;
      break;
    case function:
      delete val_function;
      val_cfunction = nullptr;
      break;
    default:
      break;
    }

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
    val_string(new string_obj(str)) {}

//  ===============
// [ String object ]
//  ===============
void string_obj::set(size_t position, const value_obj& value) {
  VIA_ASSERT(position < len, "String index position out of bounds");
  VIA_ASSERT(value.is_string(), "Setting string index to non-string value");

  const string_obj* val = value.val_string;

  VIA_ASSERT(val->len == 1, "Setting string index to non-character string");

  data[position] = value.val_string->data[0];
}

value_obj string_obj::get(size_t position) {
  VIA_ASSERT(position < len, "String index position out of bounds");
  char chr = data[position];
  string_obj* tstr = new string_obj(&chr);
  return value_obj(tstr);
}

//  ==============
// [ Array object ]
//  ==============
array_obj::array_obj(const array_obj& other)
  : capacity(other.capacity),
    size_cache(other.size_cache),
    size_cache_valid(other.size_cache_valid),
    data(new value_obj[capacity]) {
  for (size_t i = 0; i < capacity; i++) {
    data[i] = other.data[i].move();
  }
}

array_obj::array_obj(array_obj&& other)
  : capacity(other.capacity),
    size_cache(other.size_cache),
    size_cache_valid(other.size_cache_valid),
    data(other.data) {
  other.capacity = 0;
  other.size_cache = 0;
  other.size_cache_valid = false;
  other.data = nullptr;
}

array_obj& array_obj::operator=(const array_obj& other) {
  if (this != &other) {
    capacity = other.capacity;
    size_cache = other.size_cache;
    size_cache_valid = other.size_cache_valid;
    data = new value_obj[capacity];
    for (size_t i = 0; i < capacity; i++) {
      data[i] = other.data[i].move();
    }
  }

  return *this;
}

array_obj& array_obj::operator=(array_obj&& other) {
  if (this != &other) {
    capacity = other.capacity;
    size_cache = other.size_cache;
    size_cache_valid = other.size_cache_valid;
    data = other.data;

    other.capacity = 0;
    other.size_cache = 0;
    other.size_cache_valid = false;
    other.data = nullptr;
  }

  return *this;
}

size_t array_obj::size() const {
  return impl::__array_size(this);
}

value_obj& array_obj::get(size_t position) {
  return *impl::__array_get(this, position);
}

void array_obj::set(size_t position, value_obj value) {
  impl::__array_set(this, position, value.move());
}

//  ===================
// [ Dictionary object ]
//  ===================
dict_obj::dict_obj(const dict_obj& other)
  : capacity(other.capacity),
    size_cache(other.size_cache),
    size_cache_valid(other.size_cache_valid),
    data(new hash_node[capacity]) {
  for (size_t i = 0; i < capacity; ++i) {
    hash_node& src = other.data[i];
    hash_node* dst = &data[i];
    dst->key = src.key;
    dst->value = src.value.clone();
  }
}

dict_obj::dict_obj(dict_obj&& other)
  : capacity(other.capacity),
    size_cache(other.size_cache),
    size_cache_valid(other.size_cache_valid),
    data(other.data) {
  other.capacity = 0;
  other.size_cache = 0;
  other.size_cache_valid = false;
  other.data = nullptr;
}

dict_obj& dict_obj::operator=(const dict_obj& other) {
  if (this != &other) {
    capacity = other.capacity;
    size_cache = other.size_cache;
    size_cache_valid = other.size_cache_valid;
    data = new hash_node[capacity];

    for (size_t i = 0; i < capacity; ++i) {
      hash_node& src = other.data[i];
      hash_node* dst = &data[i];
      dst->key = src.key;
      dst->value = src.value.clone();
    }
  }

  return *this;
}

dict_obj& dict_obj::operator=(dict_obj&& other) {
  if (this != &other) {
    capacity = other.capacity;
    size_cache = other.size_cache;
    size_cache_valid = other.size_cache_valid;
    data = other.data;

    other.capacity = 0;
    other.size_cache = 0;
    other.size_cache_valid = false;
    other.data = nullptr;
  }

  return *this;
}

size_t dict_obj::size() const {
  return impl::__dict_size(this);
}

value_obj& dict_obj::get(const char* key) {
  return *impl::__dict_get(this, key);
}

void dict_obj::set(const char* key, value_obj value) {
  impl::__dict_set(this, key, value.move());
}

//  ===============
// [ Object object ]
//  ===============
object_obj::~object_obj() {
  if (fields) {
    delete[] fields;
    fields = nullptr;
  }
}

} // namespace via
