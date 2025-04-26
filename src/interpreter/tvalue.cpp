// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "tvalue.h"
#include "tstring.h"
#include "tarray.h"
#include "tdict.h"
#include "tfunction.h"
#include "api-impl.h"

namespace via {

using enum Value::Tag;

Value::Value()
  : type(Tag::Nil) {}

Value::Value(bool b)
  : type(Tag::Bool),
    u({.b = b}) {}

Value::Value(int x)
  : type(Tag::Int),
    u({.i = x}) {}

Value::Value(float x)
  : type(Tag::Float),
    u({.f = x}) {}

Value::Value(struct String* ptr)
  : type(Tag::String),
    u({.str = ptr}) {}

Value::Value(struct Array* ptr)
  : type(Tag::Array),
    u({.arr = ptr}) {}

Value::Value(struct Dict* ptr)
  : type(Tag::Dict),
    u({.dict = ptr}) {}

Value::Value(Closure* ptr)
  : type(Tag::Function),
    u({.clsr = ptr}) {}

// Move constructor, transfer ownership based on type
Value::Value(Value&& other)
  : type(other.type),
    u(other.u) {
  other.type = Nil;
  other.u = {};
}

// Move-assignment operator, moves values from other object
Value& Value::operator=(Value&& other) {
  if (this != &other) {
    reset();

    this->type = other.type;
    this->u = other.u;

    other.type = Nil;
    other.u = {};
  }

  return *this;
}

// Return a clone of the Value based on its type
VIA_NODISCARD Value Value::clone() const {
  switch (type) {
  case Int:
    return Value(u.i);
  case Float:
    return Value(u.f);
  case Bool:
    return Value(u.b);
  case String:
    return Value(new struct String(*u.str));
  case Array:
    return Value(new struct Array(*u.arr));
  case Dict:
    return Value(new struct Dict(*u.dict));
  case Function:
    return Value(new Closure(*u.clsr));
  default:
    break;
  }

  return Value();
}

void Value::reset() {
  switch (type) {
  case String:
    delete u.str;
    u.str = nullptr;
    break;
  case Array:
    delete u.arr;
    u.arr = nullptr;
    break;
  case Dict:
    delete u.dict;
    u.dict = nullptr;
    break;
  case Function:
    delete u.clsr;
    u.clsr = nullptr;
    break;
  default:
    break;
  }

  type = Nil;
  u = {};
}

VIA_NODISCARD bool Value::compare(const Value& other) const {
  if (type != other.type) {
    return false;
  }

  switch (type) {
  case Nil: // Nil values are always equal
    return true;
  case Int:
    return u.i == other.u.i;
  case Float:
    return u.f == other.u.f;
  case Bool:
    return u.b == other.u.b;
  case String:
    return !std::strcmp(u.str->data, other.u.str->data);
  default: // Objects are never equal with the exception of strings
    break;
  }

  return false;
}

VIA_NODISCARD Value Value::to_string() const {
  return impl::__to_string(*this);
}

VIA_NODISCARD std::string Value::to_cxx_string() const {
  return impl::__to_cxx_string(*this);
}

VIA_NODISCARD std::string Value::to_literal_cxx_string() const {
  return impl::__to_literal_cxx_string(*this);
}

} // namespace via
