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

class Value final {
 public:
  using int_type = i64;
  using float_type = f32;

  enum class Kind {
    nil,
    int_,
    float_,
    boolean,
    string,
  };

  union Union {
    i64 i;
    f64 fp;
    bool b;
    char* str;
  };

  friend class ValueRef;
  friend class Interpreter;

 public:
  static Value* construct(Interpreter* ctx);
  static Value* construct(Interpreter* ctx, int_type i);
  static Value* construct(Interpreter* ctx, float_type f);
  static Value* construct(Interpreter* ctx, bool b);
  static Value* construct(Interpreter* ctx, char* s);

 public:
  Kind kind() const;
  Union& data();
  const Union& data() const;
  Interpreter* context() const;

  void free();
  Value* clone();
  ValueRef make_ref();

  // Totally safe access methods
  int_type int_() const;
  float_type float_() const;
  bool boolean() const;
  char* string() const;

  Optional<int_type> as_cint() const;
  Optional<float_type> as_cfloat() const;
  bool as_cbool() const;
  char* as_cstring() const;

  Value* as_int() const;
  Value* as_float() const;
  Value* as_bool() const;
  Value* as_string() const;

 private:
  static Value* construct_impl(Interpreter* ctx,
                               Value::Kind kind,
                               Value::Union un = {});

 private:
  usize rc = 0;
  Kind k = Kind::nil;
  Union u = {};
  Interpreter* ctx = NULL;
};

}  // namespace via

#endif
