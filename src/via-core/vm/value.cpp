// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "value.h"

namespace via {

namespace core {

namespace vm {

constexpr Optional<Value::int_type> Value::as_cint() const {
  switch (kind) {
    case Int:
      return u.i;
    case Float:
      return static_cast<int_type>(u.fp);
    case Bool:
      return static_cast<int_type>(u.b);
    case String: {
      auto result = stoi<int_type>(u.str);
      if (result.has_value())
        return *result;
      break;
    }
    default:
      break;
  }

  return nullopt;
}

constexpr Optional<Value::float_type> Value::as_cfloat() const {
  switch (kind) {
    case Float:
      return u.fp;
    case Int:
      return static_cast<float_type>(u.i);
    case Bool:
      return static_cast<float_type>(u.b);
    case String: {
      auto result = stof<float_type>(u.str);
      if (result.has_value())
        return *result;
      break;
    }
    default:
      break;
  }

  return nullopt;
}

constexpr Value::bool_type Value::as_cbool() const {
  switch (kind) {
    case Bool:
      return u.b;
    case Int:
      return static_cast<bool_type>(u.i);
    case Float:
      return static_cast<bool_type>(u.fp);
    case String:
      return true;
    default:
      break;
  }

  return false;
}

constexpr char* Value::as_cstring() const {
  VIA_TODO("as_cstring not implemented")
}

constexpr Value Value::as_int() const {
  auto result = as_cint();
  return result.has_value() ? Value(ctx, *result) : Value(ctx);
}

constexpr Value Value::as_float() const {
  auto result = as_cint();
  return result.has_value() ? Value(ctx, *result) : Value(ctx);
}

constexpr Value Value::as_bool() const {
  return Value(ctx, as_cbool());
}

constexpr Value Value::as_string() const {
  return Value(ctx, as_cstring());
}

}  // namespace vm

}  // namespace core

}  // namespace via
