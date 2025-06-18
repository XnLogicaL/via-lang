// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmstr.h"
#include "vmstate.h"

namespace via {

namespace vm {

String string_new(State* S, const char* str) {
  size_t len = std::strlen(str);
  if (len > VIA_MAXSSIZE)
    error_toobig(S);

  String lstr;
  lstr.data = StrBuf(len + 1);
  lstr.hash = ustrhash(str);

  std::strcpy(lstr.data.data, str);

  return lstr;
}

char string_get(State* S, String* str, size_t pos) {
  if (pos > str->data.size)
    error_outofbounds(S);

  return str->data.data[pos];
}

void string_set(State* S, String* str, size_t pos, char chr) {
  if (pos > str->data.size)
    error_outofbounds(S);

  str->data.data[pos] = chr;
}

bool string_cmp(State*, String* left, String* right) {
  if (left->data.size != right->data.size)
    return false;

  return std::strcmp(left->data.data, right->data.data);
}

} // namespace vm

} // namespace via
