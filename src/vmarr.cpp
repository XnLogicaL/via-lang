// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmarr.h"
#include "vmiapi.h"

namespace via {

Array::Array(const Array& other)
  : data(new Value[ARRAY_INITAL_CAPACITY]),
    data_capacity(other.data_capacity),
    csize(other.csize) {
  for (size_t i = 0; i < data_capacity; i++) {
    data[i] = other.data[i].clone();
  }
}

Array::Array(Array&& other)
  : data(other.data),
    data_capacity(other.data_capacity),
    csize(other.csize) {
  other.data_capacity = 0;
  other.data = nullptr;
  other.csize = {};
}

Array& Array::operator=(const Array& other) {
  if (this != &other) {
    delete[] data;

    data = new Value[data_capacity];
    data_capacity = other.data_capacity;
    csize = other.csize;

    for (size_t i = 0; i < data_capacity; i++) {
      data[i] = other.data[i].clone();
    }
  }

  return *this;
}

Array& Array::operator=(Array&& other) {
  if (this != &other) {
    delete[] data;

    data = other.data;
    data_capacity = other.data_capacity;
    csize = other.csize;

    other.data_capacity = 0;
    other.csize = {};
    other.data = nullptr;
  }

  return *this;
}

Array::Array()
  : data(new Value[ARRAY_INITAL_CAPACITY]) {}

Array::~Array() {
  delete[] data;
}

size_t Array::size() const {
  return impl::__array_size(this);
}

Value& Array::get(size_t position) {
  return *impl::__array_get(this, position);
}

void Array::set(size_t position, Value&& value) {
  impl::__array_set(this, position, std::move(value));
}

} // namespace via
