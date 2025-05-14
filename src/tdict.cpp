// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "tdict.h"
#include "api-impl.h"

namespace via {

Dict::Dict(const Dict& other)
  : data(new Dict::HNode[DICT_INITIAL_CAPACITY]),
    data_capacity(other.data_capacity),
    csize(other.csize) {
  for (size_t i = 0; i < data_capacity; ++i) {
    HNode& src = other.data[i];
    HNode* dst = &data[i];
    dst->key = src.key;
    dst->value = src.value.clone();
  }
}

Dict::Dict(Dict&& other)
  : data(other.data),
    data_capacity(other.data_capacity),
    csize(other.csize) {
  other.data = nullptr;
  other.data_capacity = 0;
  other.csize = {};
}

Dict& Dict::operator=(const Dict& other) {
  if (this != &other) {
    data = new HNode[other.data_capacity];
    data_capacity = other.data_capacity;
    csize = other.csize;

    for (size_t i = 0; i < data_capacity; ++i) {
      Dict::HNode& src = other.data[i];
      Dict::HNode* dst = &data[i];
      dst->key = src.key;
      dst->value = src.value.clone();
    }
  }

  return *this;
}

Dict& Dict::operator=(Dict&& other) {
  if (this != &other) {
    data = other.data;
    data_capacity = other.data_capacity;
    csize = other.csize;

    other.data = nullptr;
    other.data_capacity = 0;
    other.csize = {};
  }

  return *this;
}

Dict::Dict()
  : data(new HNode[DICT_INITIAL_CAPACITY]) {}

Dict::~Dict() {
  delete[] data;
}

size_t Dict::size() const {
  return impl::__dict_size(this);
}

Value& Dict::get(const char* key) {
  return *impl::__dict_get(this, key);
}

void Dict::set(const char* key, Value value) {
  impl::__dict_set(this, key, std::move(value));
}

} // namespace via
