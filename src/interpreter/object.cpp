//  ========================================================================================
// [ This file is a part of The via Programming Language and is licensed under GNU GPL v3.0 ]
//  ========================================================================================
#include "api-impl.h"
#include "string-utility.h"
#include "object.h"
#include "function.h"
#include "api-aux.h"

namespace via {

using enum IValueType;

//  ==============
// [ Value object ]
//  ==============

// Move-assignment operator, moves values from other object
IValue& IValue::operator=(IValue&& other) {
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
    other.type = IValueType::nil;
  }

  return *this;
}

// Move constructor, transfer ownership based on type
IValue::IValue(IValue&& other) {
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

// Return a clone of the IValue based on its type
IValue IValue::clone() const {
  switch (type) {
  case integer:
    return IValue(val_integer);
  case floating_point:
    return IValue(val_floating_point);
  case boolean:
    return IValue(val_boolean);
  case string:
    return IValue(val_string->data);
  case array:
    return IValue(new IArray(*val_array));
  case dict:
    return IValue(new IDict(*val_dict));
  case function:
    return IValue(new IFunction(*val_function));
  case cfunction:
    return IValue(val_cfunction);
  default:
    break;
  }

  return IValue();
}

void IValue::reset() {
  if (static_cast<uint8_t>(type) >= static_cast<uint8_t>(IValueType::string)) {
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

bool IValue::compare(const IValue& other) const {
  if (type != other.type) {
    return false;
  }

  switch (type) {
  case nil: // nil values are always equal
    return true;
  case integer:
    return val_integer == other.val_integer;
  case floating_point:
    return val_floating_point == other.val_floating_point;
  case boolean:
    return val_boolean == other.val_boolean;
  case string:
    return !std::strcmp(val_string->data, other.val_string->data);
  default: // Objects are never equal with the exception of strings
    break;
  }

  return false;
}

IValue IValue::to_string() const {
  return impl::__to_string(*this);
}

std::string IValue::to_cxx_string() const {
  return impl::__to_cxx_string(*this);
}

std::string IValue::to_literal_cxx_string() const {
  return impl::__to_literal_cxx_string(*this);
}

IValue::IValue(const char* str)
  : type(string),
    val_string(new IString(str)) {}

//  ===============
// [ String object ]
//  ===============
void IString::set(size_t position, const IValue& value) {
  VIA_ASSERT(position < len, "String index position out of bounds");
  VIA_ASSERT(value.is_string(), "Setting string index to non-string value");

  const IString* val = value.val_string;

  VIA_ASSERT(val->len == 1, "Setting string index to non-character string");

  data[position] = value.val_string->data[0];
}

IValue IString::get(size_t position) {
  VIA_ASSERT(position < len, "String index position out of bounds");
  char chr = data[position];
  IString* tstr = new IString(&chr);
  return IValue(tstr);
}

//  ==============
// [ Array object ]
//  ==============
IArray::IArray(const IArray& other)
  : capacity(other.capacity),
    size_cache(other.size_cache),
    size_cache_valid(other.size_cache_valid),
    data(new IValue[capacity]) {
  for (size_t i = 0; i < capacity; i++) {
    data[i] = other.data[i].clone();
  }
}

IArray::IArray(IArray&& other)
  : capacity(other.capacity),
    size_cache(other.size_cache),
    size_cache_valid(other.size_cache_valid),
    data(other.data) {
  other.capacity = 0;
  other.size_cache = 0;
  other.size_cache_valid = false;
  other.data = nullptr;
}

IArray& IArray::operator=(const IArray& other) {
  if (this != &other) {
    delete[] data;

    capacity = other.capacity;
    size_cache = other.size_cache;
    size_cache_valid = other.size_cache_valid;
    data = new IValue[capacity];

    for (size_t i = 0; i < capacity; i++) {
      data[i] = other.data[i].clone();
    }
  }

  return *this;
}

IArray& IArray::operator=(IArray&& other) {
  if (this != &other) {
    delete[] data;

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

size_t IArray::size() const {
  return impl::__array_size(this);
}

IValue& IArray::get(size_t position) {
  return *impl::__array_get(this, position);
}

void IArray::set(size_t position, IValue value) {
  impl::__array_set(this, position, std::move(value));
}

//  ===================
// [ Dictionary object ]
//  ===================
IDict::IDict(const IDict& other)
  : capacity(other.capacity),
    size_cache(other.size_cache),
    size_cache_valid(other.size_cache_valid),
    data(new IHashNode[capacity]) {
  for (size_t i = 0; i < capacity; ++i) {
    IHashNode& src = other.data[i];
    IHashNode* dst = &data[i];
    dst->key = src.key;
    dst->value = src.value.clone();
  }
}

IDict::IDict(IDict&& other)
  : capacity(other.capacity),
    size_cache(other.size_cache),
    size_cache_valid(other.size_cache_valid),
    data(other.data) {
  other.capacity = 0;
  other.size_cache = 0;
  other.size_cache_valid = false;
  other.data = nullptr;
}

IDict& IDict::operator=(const IDict& other) {
  if (this != &other) {
    capacity = other.capacity;
    size_cache = other.size_cache;
    size_cache_valid = other.size_cache_valid;
    data = new IHashNode[capacity];

    for (size_t i = 0; i < capacity; ++i) {
      IHashNode& src = other.data[i];
      IHashNode* dst = &data[i];
      dst->key = src.key;
      dst->value = src.value.clone();
    }
  }

  return *this;
}

IDict& IDict::operator=(IDict&& other) {
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

size_t IDict::size() const {
  return impl::__dict_size(this);
}

IValue& IDict::get(const char* key) {
  return *impl::__dict_get(this, key);
}

void IDict::set(const char* key, IValue value) {
  impl::__dict_set(this, key, std::move(value));
}

//  ===============
// [ Object object ]
//  ===============
IObject::~IObject() {
  if (fields) {
    delete[] fields;
    fields = nullptr;
  }
}

} // namespace via
