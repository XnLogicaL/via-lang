// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "value.h"

namespace via
{

Value* Value::constructImpl(Interpreter* ctx,
                            Value::Kind kind,
                            Value::Union data)
{
  Value* ptr = ctx->getAllocator().emplace<Value>();
  ptr->mKind = kind;
  ptr->mData = data;
  return ptr;
}

Value* Value::construct(Interpreter* ctx)
{
  return constructImpl(ctx, Kind::Nil);
}

Value* Value::construct(Interpreter* ctx, Value::int_type int_)
{
  return constructImpl(ctx, Kind::Int, {.int_ = int_});
}

Value* Value::construct(Interpreter* ctx, Value::float_type float_)
{
  return constructImpl(ctx, Kind::Float, {.float_ = float_});
}

Value* Value::construct(Interpreter* ctx, bool boolean)
{
  return constructImpl(ctx, Kind::Boolean, {.boolean = boolean});
}

Value* Value::construct(Interpreter* ctx, char* string)
{
  debug::assertm(
      ctx->getAllocator().owns(string),
      "Value construction via a string literal requires it to be allocated by "
      "the corresponding Value::ctx");
  return constructImpl(ctx, Kind::std::string, {.string = string});
}

void Value::free()
{
  switch (mKind) {
    case Kind::std::string:
      mCtx->getAllocator().free(mData.string);
      break;
    default:
      // Trivial types don't require explicit destruction
      break;
  }

  mKind = Kind::Nil;
}

Value* Value::clone()
{
  return constructImpl(mCtx, mKind, mData);
}

ValueRef Value::makeRef()
{
  return ValueRef(mCtx, this);
}

}  // namespace via
