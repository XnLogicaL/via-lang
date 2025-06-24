// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VAL_H
#define VIA_VAL_H

#include <common/common.h>

namespace via {

struct State;
struct String;
struct Array;
struct Dict;
struct Closure;
struct Function;

enum ValueKind : uint8_t {
  VLK_NIL,
  VLK_INT,
  VLK_FLOAT,
  VLK_BOOLEAN,
  VLK_STRING,
  VLK_FUNCTION,
  VLK_ARRAY,
  VLK_DICT,
};

union ValueUn {
  int i;
  float f;
  bool b;
  String* str;
  Closure* clsr;
};

struct ValueData {
  int rc;
  ValueUn u;
};

struct alignas(8) Value {
  ValueKind kind = VLK_NIL;
  ValueData* data = NULL;
  State* S = NULL;
};

Value value_new(State* S);
Value value_new(State* S, int i);
Value value_new(State* S, float f);
Value value_new(State* S, bool b);
Value value_new(State* S, const char* str);
Value value_new(State* S, Function* F);
Value value_new(State* S, ValueKind kind, ValueData* data);

void value_close(State* S, Value* value);
Value value_clone(State* S, const Value* other);
Value value_ref(State* S, const Value* other);
void value_reset(State* S, Value* value);
bool value_cmp(State* S, const Value* left, const Value* right);

inline const Value nil;

} // namespace via

#endif