// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "value.h"

namespace via {

Value* Value::construct_impl(Interpreter* ctx,
                             Value::Kind kind,
                             Value::Union data) {
  Value* ptr = ctx->get_allocator().emplace<Value>();
  ptr->k = kind;
  ptr->u = data;
  return ptr;
}

Value* Value::construct(Interpreter* ctx) {
  return construct_impl(ctx, Kind::nil);
}

Value* Value::construct(Interpreter* ctx, Value::int_type int_) {
  return construct_impl(ctx, Kind::int_, {.int_ = int_});
}

Value* Value::construct(Interpreter* ctx, Value::float_type float_) {
  return construct_impl(ctx, Kind::float_, {.float_ = float_});
}

Value* Value::construct(Interpreter* ctx, bool boolean) {
  return construct_impl(ctx, Kind::boolean, {.boolean = boolean});
}

Value* Value::construct(Interpreter* ctx, char* string) {
  assert(
      ctx->get_allocator().owns(string) &&
      "Value construction via a string literal requires it to be allocated by "
      "the corresponding Value::ctx");
  return construct_impl(ctx, Kind::string, {.string = string});
}

void Value::free() {
  switch (k) {
    case Kind::string:
      ctx->get_allocator().free(u.string);
      break;
    default:
      // Trivial types don't require explicit destruction
      break;
  }

  k = Kind::nil;
}

Value* Value::clone() {
  return construct_impl(ctx, k, u);
}

ValueRef Value::make_ref() {
  return ValueRef(ctx, this);
}

}  // namespace via
