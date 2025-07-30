// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_VALUE_H_
#define VIA_VM_VALUE_H_

#include <util/constexpr_stof.h>
#include <util/constexpr_stoi.h>
#include <via/config.h>
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
  using bool_type = bool;

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

  constexpr Optional<int_type> as_cint() const;
  constexpr Optional<float_type> as_cfloat() const;
  constexpr bool_type as_cbool() const;
  constexpr char* as_cstring() const;

  constexpr Value as_int() const;
  constexpr Value as_float() const;
  constexpr Value as_bool() const;
  constexpr Value as_string() const;

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

  constexpr Value arith_add(const Value* other) const;
  constexpr Value arith_sub(const Value* other) const;
  constexpr Value arith_mul(const Value* other) const;
  constexpr Value arith_div(const Value* other) const;
  constexpr Value arith_pow(const Value* other) const;
  constexpr Value arith_mod(const Value* other) const;

  template <const Arith At>
  inline constexpr Value arith(const Value* other) const {
    switch (At) {
      case Arith::Add:
        return arith_add(other);
      case Arith::Sub:
        return arith_sub(other);
      case Arith::Mul:
        return arith_mul(other);
      case Arith::Div:
        return arith_div(other);
      case Arith::Pow:
        return arith_pow(other);
      case Arith::Mod:
        return arith_mod(other);
      default:
        break;
    }

    VIA_BUG("invalid Arith enum value");
  }

#define CTOR inline constexpr explicit Value
#define TYPED_CTOR(type, ctype, id) \
  CTOR(Interpreter* ctx, ctype id) : ctx(ctx), kind(type), u({.id = id}) {}

  CTOR(Interpreter* ctx) : ctx(ctx) {}
  TYPED_CTOR(Int, int_type, i);
  TYPED_CTOR(Float, float_type, fp);
  TYPED_CTOR(Bool, bool_type, b);
  TYPED_CTOR(String, char*, str);

#undef CTOR
#undef TYPED_CTOR
};

}  // namespace vm

}  // namespace core

}  // namespace via

#endif
