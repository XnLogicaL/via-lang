// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "value.h"

namespace via {

Value* Value::construct_impl(Interpreter* ctx,
                             Value::Kind kind,
                             Value::Union un) {
  Value* ptr = ctx->get_allocator().emplace<Value>();
  ptr->k = kind;
  ptr->u = un;
  return ptr;
}

Value* Value::construct(Interpreter* ctx) {
  return construct_impl(ctx, Kind::nil);
}

Value* Value::construct(Interpreter* ctx, Value::int_type i) {
  return construct_impl(ctx, Kind::int_, {.i = i});
}

Value* Value::construct(Interpreter* ctx, Value::float_type fp) {
  return construct_impl(ctx, Kind::float_, {.fp = fp});
}

Value* Value::construct(Interpreter* ctx, bool b) {
  return construct_impl(ctx, Kind::boolean, {.b = b});
}

Value* Value::construct(Interpreter* ctx, char* s) {
  assert(
      ctx->get_allocator().owns(s) &&
      "Value construction via a string literal requires it to be allocated by "
      "the corresponding Value::ctx");
  return construct_impl(ctx, Kind::string, {.str = s});
}

Value::Kind Value::kind() const {
  return k;
}

Value::Union& Value::data() {
  return u;
}

const Value::Union& Value::data() const {
  return u;
}

Interpreter* Value::context() const {
  return ctx;
}

void Value::free() {
  switch (k) {
    case Kind::string:
      ctx->get_allocator().free(u.str);
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
