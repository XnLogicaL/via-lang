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

Value::Value(bool owns, Tag ty, Un un)
  : owns(owns),
    type(ty),
    u(un) {}

Value::Value(bool owns)
  : owns(owns),
    type(Tag::Nil) {}

Value::Value(bool b, bool owns)
  : owns(owns),
    type(Tag::Bool),
    u({.b = b}) {}

Value::Value(int x, bool owns)
  : owns(owns),
    type(Tag::Int),
    u({.i = x}) {}

Value::Value(float x, bool owns)
  : owns(owns),
    type(Tag::Float),
    u({.f = x}) {}

Value::Value(struct String* ptr, bool owns)
  : owns(owns),
    type(Tag::String),
    u({.str = ptr}) {}

Value::Value(struct Array* ptr, bool owns)
  : owns(owns),
    type(Tag::Array),
    u({.arr = ptr}) {}

Value::Value(struct Dict* ptr, bool owns)
  : owns(owns),
    type(Tag::Dict),
    u({.dict = ptr}) {}

Value::Value(Closure* ptr, bool owns)
  : owns(owns),
    type(Tag::Function),
    u({.clsr = ptr}) {}

Value::Value(Tag t, Un u, bool owns)
  : owns(owns),
    type(t),
    u(u) {}

Value::Value(const char* str)
  : type(String),
    u({.str = new struct String(str)}) {}

// Move constructor, transfer ownership based on type
Value::Value(Value&& other)
  : owns(true) {
  reset();

  this->type = other.type;
  this->u = other.u;
  other.type = Nil;
}

// Move-assignment operator, moves values from other object
Value& Value::operator=(Value&& other) {
  if (this != &other) {
    reset();

    this->type = other.type;
    this->u = other.u;
    other.type = Tag::Nil;
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
    return Value(u.b, true);
  case String:
    return Value(u.str->data);
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

VIA_NODISCARD Value Value::weak_clone() const {
  return Value(type, u, false);
}

void Value::reset() {
  if (owns && static_cast<uint8_t>(type) >= static_cast<uint8_t>(Tag::String)) {
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
  }

  type = Nil;
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

VIA_NODISCARD bool Value::is_reference(const Value& other) const {
  if (other.owns || other.type != this->type) {
    return false;
  }

  return std::memcmp(&this->u, &other.u, sizeof(Un));
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
