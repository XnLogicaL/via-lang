// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#include "string-utility.h"
#include "object.h"
#include "function.h"
#include "api-aux.h"

namespace via {

using enum value_type;

// Move-assignment operator, moves values from other object
value_obj& value_obj::operator=(value_obj&& other) {
  if (this != &other) {
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

    type = other.type;
    other.type = nil;
  }

  return *this;
}

// Move constructor, transfer ownership based on type
value_obj::value_obj(value_obj&& other)
  : type(other.type) {
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

  other.type = nil;
}

// Frees the resources of the value_obj depending on type
value_obj::~value_obj() {
  if (static_cast<uint16_t>(type) >= static_cast<uint8_t>(value_type::string)) {
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
      delete cast_ptr<tfunction>();
      break;
    case cfunction:
      delete cast_ptr<tcfunction>();
      break;
    default:
      break;
    }

    val_pointer = nullptr;
    type = nil;
  }
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
    return value_obj(string, new string_obj(*cast_ptr<string_obj>()));
  case table:
    return value_obj(table, new table_obj(*cast_ptr<table_obj>()));
  case function:
    return value_obj(function, new tfunction(*cast_ptr<tfunction>()));
  case cfunction:
    return value_obj(cfunction, new tcfunction(*cast_ptr<tcfunction>()));
  default:
    return value_obj();
  }

  vl_unreachable;
}

void value_obj::reset() {}

bool value_obj::compare(const value_obj&) const {
  return false;
}

// Constructs a new string_obj object
string_obj::string_obj(State* V, const char* str) {
  if (V != nullptr) {
    auto& stable = V->G->stable;
    auto it = stable.find(hash);
    if (it != stable.end()) {
      return;
    }
  }

  len = std::strlen(str);
  data = duplicate_string(str);
  hash = hash_string_custom(str);

  if (V != nullptr) {
    V->G->stable.emplace(hash, this);
  }
}

string_obj::string_obj(const string_obj& other)
  : len(other.len),
    hash(other.hash) {
  data = duplicate_string(other.data);
}

string_obj::~string_obj() {
  delete[] data;
}

size_t string_obj::size() {
  return len;
}

void string_obj::set_string(size_t position, const value_obj& value) {
  vl_assert(position < len, "String index position out of bounds");
  vl_assert(value.is_string(), "Setting string index to non-string value");

  const string_obj* val = value.cast_ptr<string_obj>();

  vl_assert(val->len == 1, "Setting string index to non-character string");

  data[position] = value.cast_ptr<string_obj>()->data[0];
}

value_obj string_obj::get_string(size_t position) {
  vl_assert(position < len, "String index position out of bounds");
  char chr = data[position];
  string_obj* tstr = new string_obj(nullptr, &chr);
  return value_obj(tstr);
}

table_obj::~table_obj() {
  if (ht_buckets) {
    for (size_t i = 0; i < ht_capacity; ++i) {
      hash_node_obj* next = ht_buckets[i];
      while (next) {
        hash_node_obj* current = next;
        next = next->next;
        delete current;
      }
      ht_buckets[i] = nullptr;
    }
  }

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
    ht_buckets = new hash_node_obj*[ht_capacity]();

    for (size_t i = 0; i < ht_capacity; ++i) {
      hash_node_obj* src = other.ht_buckets[i];
      hash_node_obj** dst = &ht_buckets[i];

      while (src) {
        *dst = new hash_node_obj{src->key, src->value.clone(), nullptr};
        dst = &((*dst)->next);
        src = src->next;
      }
    }
  }
  else {
    ht_buckets = nullptr;
  }
}

size_t table_obj::size() {
  return impl::__table_size(this);
}

void table_obj::set_table(const char* key, const value_obj& value) {
  value_obj index(new string_obj(nullptr, key));
  impl::__table_set(this, index, value);
}

void table_obj::set_table(size_t position, const value_obj& value) {
  value_obj index(static_cast<TInteger>(position));
  impl::__table_set(this, index, value);
}

value_obj table_obj::get_table(const char* key) {
  value_obj index(new string_obj(nullptr, key));
  return impl::__table_get(this, index);
}

value_obj table_obj::get_table(size_t position) {
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
