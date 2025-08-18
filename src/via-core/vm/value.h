// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_VALUE_H_
#define VIA_VM_VALUE_H_

#include <via/config.h>
#include <via/types.h>
#include "interpreter.h"
#include "value_ref.h"

namespace via {

class Interpreter;

class Value final {
 public:
  using int_type = i64;
  using float_type = f32;

  enum class Kind {
    Nil,
    Int,
    Float,
    Boolean,
    String,
  };

  union Union {
    int_type int_;
    float_type float_;
    bool boolean;
    char* string;
  };

  friend class ValueRef;
  friend class Interpreter;

 public:
  static Value* construct(Interpreter* ctx);
  static Value* construct(Interpreter* ctx, int_type int_);
  static Value* construct(Interpreter* ctx, float_type float_);
  static Value* construct(Interpreter* ctx, bool boolean);
  static Value* construct(Interpreter* ctx, char* string);

 public:
  Kind kind() const { return k; }
  Union& data() { return u; }
  const Union& data() const { return u; }
  Interpreter* context() const { return ctx; }

  void free();
  Value* clone();
  ValueRef make_ref();

  // Totally safe access methods
  int_type get_int() const { return u.int_; }
  float_type get_float() const { return u.float_; }
  bool get_boolean() const { return u.boolean; }
  char* get_string() const { return u.string; }

  Optional<int_type> as_cint() const;
  Optional<float_type> as_cfloat() const;
  bool as_cbool() const;
  char* as_cstring() const;

  Value* as_int() const;
  Value* as_float() const;
  Value* as_bool() const;
  Value* as_string() const;

 private:
  static Value* construct_impl(Interpreter* ctx, Kind kind, Union data = {});

 private:
  Kind k = Kind::Nil;
  Union u = {};
  usize rc = 0;
  Interpreter* ctx = NULL;
};

}  // namespace via

#endif
