// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_VALUE_H_
#define VIA_VM_VALUE_H_

#include <via/config.h>
#include <via/types.h>
#include "constexpr_stof.h"
#include "constexpr_stoi.h"
#include "interpreter.h"
#include "value_ref.h"

namespace via {

class Interpreter;
struct PseudoValue;

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
  Value* clone();
  ValueRef make_ref();

  Optional<int_type> as_cint() const;
  Optional<float_type> as_cfloat() const;
  bool as_cbool() const;
  char* as_cstring() const;

  Value* as_int() const;
  Value* as_float() const;
  Value* as_bool() const;
  Value* as_string() const;

  template <Kind As>
  inline Value* as() const {
    return NULL;
  }

  static Value* make(Interpreter* ctx);
  static Value* make(Interpreter* ctx, int_type i);
  static Value* make(Interpreter* ctx, float_type f);
  static Value* make(Interpreter* ctx, bool b);
  static Value* make(Interpreter* ctx, char* s);

  static Value* from_pseudo(Interpreter* ctx, const PseudoValue& psv);
};

}  // namespace via

#endif
