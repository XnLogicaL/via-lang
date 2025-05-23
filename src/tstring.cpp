// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "tstring.h"
#include "api-impl.h"

namespace via {

String::String(const char* str)
  : data(ustrdup(str)),
    data_size(std::strlen(str)),
    hash(ustrhash(str)) {}

String::~String() {
  delete[] data;
}

String::String(const String& other)
  : data(ustrdup(other.data)),
    data_size(other.data_size),
    hash(other.hash) {}

String::String(String&& other)
  : data(other.data),
    data_size(other.data_size),
    hash(other.hash) {
  other.data = nullptr;
  other.data_size = 0;
  other.hash = 0;
}

String& String::operator=(const String& other) {
  if (this != &other) {
    delete[] data;

    data = ustrdup(other.data);
    data_size = other.data_size;
    hash = other.hash;
  }

  return *this;
}

String& String::operator=(String&& other) {
  if (this != &other) {
    delete[] data;

    data = other.data;
    data_size = other.data_size;
    hash = other.hash;

    other.data = nullptr;
    other.data_size = 0;
    other.hash = 0;
  }

  return *this;
}

void String::set(size_t position, const String& value) {
  VIA_ASSERT(position < data_size, "String index position out of bounds");
  VIA_ASSERT(value.data_size == 1, "Setting String index to non-character String");

  data[position] = value.data[0];
}

String String::get(size_t position) {
  VIA_ASSERT(position < data_size, "String index position out of bounds");
  char chr = data[position];
  char str[] = {chr, '\0'};
  return String(str);
}

} // namespace via
