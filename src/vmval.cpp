// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "vmval.h"
#include "vmstate.h"
#include "vmstr.h"

namespace via {

static ValueData* clone_data(const ValueData&) {
  return NULL;
}

Value value_new(State* S) {
  Value val;
  val.kind = VLK_NIL;
  val.data = new ValueData;
  val.S = S;

  val.data->rc++;

  return val;
}

Value value_new(State* S, ValueKind kind, ValueData* data) {
  Value val;
  val.kind = kind;
  val.data = data;
  val.S = S;

  data->rc++;

  return val;
}

void value_close(State*, Value* value) {
  value->kind = VLK_NIL;
  value->data->rc--;

  if (value->data->rc == 0)
    delete value->data;
}

Value value_clone(State*, const Value* value) {
  return value_new(value->S, value->kind, clone_data(*value->data));
}

Value value_ref(State*, const Value* value) {
  return value_new(value->S, value->kind, value->data);
}

void value_reset(State*, Value* value) {
  value->kind = VLK_NIL;
  value->data->rc--;
  if (value->data->rc == 0) {
    delete value->data;
  }
}

bool value_cmp(State* S, const Value* left, const Value* right) {
  if (left == right) {
    return true;
  }

  if (left->data == right->data) {
    return true;
  }

  if (left->kind != right->kind) {
    return false;
  }

  switch (left->kind) {
  case VLK_NIL:
    return true;
  case VLK_BOOLEAN:
    return left->data->u.b == right->data->u.b;
  case VLK_INT:
    return left->data->u.i == right->data->u.i;
  case VLK_FLOAT:
    return left->data->u.f == right->data->u.f;
  case VLK_STRING:
    return string_cmp(S, left->data->u.str, right->data->u.str);
  default:
    break;
  }

  return false;
}

} // namespace via
