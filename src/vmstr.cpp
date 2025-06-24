// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmstr.h"
#include "vmstate.h"

namespace via {

String string_new(State* S, const char* str) {
  size_t len = std::strlen(str);
  if (len > VIA_MAXSSIZE) {
    error(S, "memory allocation failed: block too large");
    return {};
  }

  String lstr;
  lstr.size = len;
  lstr.data = (char*)S->ator.alloc_bytes(len + 1);
  lstr.hash = ustrhash(str);

  std::strcpy(lstr.data, str);

  return lstr;
}

char string_get(State* S, String* str, size_t pos) {
  if (pos > str->size) {
    error(S, "string index out of range");
    return -1;
  }

  return str->data[pos];
}

void string_set(State* S, String* str, size_t pos, char chr) {
  if (pos > str->size) {
    error(S, "string index out of range");
    return;
  }

  str->data[pos] = chr;
}

bool string_cmp(State*, String* left, String* right) {
  if (left->size != right->size)
    return false;

  return std::strcmp(left->data, right->data);
}

} // namespace via
