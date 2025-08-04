// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_VALUE_H_
#define VIA_VM_VALUE_H_

#include <util/constexpr_stof.h>
#include <util/constexpr_stoi.h>
#include <via/config.h>
#include "interpreter.h"
#include "value_ref.h"

namespace via {

namespace core {

namespace vm {

class Interpreter;

enum class Arith {
  Add,
  Sub,
  Mul,
  Div,
  Pow,
  Mod,
};

struct Value {
  using int_type = i64;
  using float_type = f32;

  usize rc = 0;
  Interpreter* ctx;

  enum Kind {
    Nil,
    Int,
    Float,
    Bool,
    String,
  } kind = Nil;

  union Un {
    i64 i;
    f64 fp;
    bool b;
    char* str;
  } u;

  void free();
  ValueRef make_ref();

  constexpr Optional<int_type> as_cint() const {
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

  constexpr Optional<float_type> as_cfloat() const {
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

  constexpr bool as_cbool() const {
    switch (kind) {
      case Bool:
        return u.b;
      case Int:
        return static_cast<bool>(u.i);
      case Float:
        return static_cast<bool>(u.fp);
      case String:
        return true;
      default:
        break;
    }

    return false;
  }

  constexpr char* as_cstring() const {
#define ALLOC_STR(str) ctx->get_allocator().strdup(str)

    switch (kind) {
      case String:
        return ALLOC_STR(u.str);
      case Nil:
        return ALLOC_STR("nil");
      case Bool:
        return ALLOC_STR(u.b ? "true" : "false");
      case Int:
        return ALLOC_STR(std::to_string(u.i).c_str());
      case Float:
        return ALLOC_STR(std::to_string(u.fp).c_str());
      default:
        break;
    }

    VIA_BUG("bad kind value");
#undef ALLOC_STR
  }

  constexpr Value as_int() const {
    auto result = as_cint();
    return result.has_value() ? Value(ctx, *result) : Value(ctx);
  }

  constexpr Value as_float() const {
    auto result = as_cint();
    return result.has_value() ? Value(ctx, *result) : Value(ctx);
  }

  constexpr Value as_bool() const {
    return Value(ctx, as_cbool());
  }

  constexpr Value as_string() const {
    return Value(ctx, as_cstring());
  }

  template <const Kind As>
  inline constexpr Value as() const {
    switch (As) {
      case Int:
        return as_int();
      case Float:
        return as_float();
      case Bool:
        return as_bool();
      default:
        break;
    }

    return Value(ctx);
  }

#define CTOR inline constexpr explicit Value
#define TYPED_CTOR(type, ctype, id) \
  CTOR(Interpreter* ctx, ctype id) : ctx(ctx), kind(type), u({.id = id}) {}

  CTOR(Interpreter* ctx) : ctx(ctx) {}
  TYPED_CTOR(Int, int_type, i);
  TYPED_CTOR(Float, float_type, fp);
  TYPED_CTOR(Bool, bool, b);
  TYPED_CTOR(String, char*, str);

#undef CTOR
#undef TYPED_CTOR
};

}  // namespace vm

}  // namespace core

}  // namespace via

#endif
