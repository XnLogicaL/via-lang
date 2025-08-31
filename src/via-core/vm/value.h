// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_VALUE_H_
#define VIA_VM_VALUE_H_

#include <via/config.h>
#include <via/types.h>
#include "interpreter.h"
#include "value_ref.h"

namespace via
{

class Interpreter;

class Value final
{
 public:
  using int_type = i64;
  using float_type = f32;

  enum class Kind
  {
    Nil,
    Int,
    Float,
    Boolean,
    String,
  };

  union Union
  {
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
  Kind kind() const { return mKind; }
  Union& data() { return mData; }
  const Union& data() const { return mData; }
  Interpreter* context() const { return mCtx; }

  void free();
  Value* clone();
  ValueRef makeRef();

  // Totally safe access methods
  int_type getInt() const { return mData.int_; }
  float_type getFloat() const { return mData.float_; }
  bool getBool() const { return mData.boolean; }
  char* getString() const { return mData.string; }

  Optional<int_type> asCInt() const;
  Optional<float_type> asCFloat() const;
  bool asCBool() const;
  char* asCString() const;

  Value* asInt() const;
  Value* asFloat() const;
  Value* asBool() const;
  Value* asString() const;

 private:
  static Value* constructImpl(Interpreter* mCtx, Kind kind, Union data = {});

 private:
  Kind mKind = Kind::Nil;
  Union mData = {};
  usize mRc = 0;
  Interpreter* mCtx = nullptr;
};

}  // namespace via

#endif
