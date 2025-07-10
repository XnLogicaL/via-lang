// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_STRING_H
#define VIA_STRING_H

#include "common.h"
#include "strutils.h"

#define VIA_MAXSSIZE (1 << 16)

namespace via {

struct State;

struct String {
  char* data = NULL;
  usize size;
  u32 hash;
};

String string_new(State* S, const char* str);
char string_get(State* S, String* str, usize pos);
void string_set(State* S, String* str, usize pos, char chr);
bool string_cmp(State* S, String* left, String* right);

} // namespace via

#endif
