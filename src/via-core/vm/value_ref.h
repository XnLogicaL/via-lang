// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_VALUE_REF_H_
#define VIA_VM_VALUE_REF_H_

#include <via/config.h>
#include <via/types.h>

namespace via
{

class Interpreter;
class Value;

class ValueRef final
{
 public:
  ValueRef(Interpreter* ctx);
  ValueRef(Interpreter* ctx, Value* ptr);
  ValueRef(const ValueRef& other);
  ValueRef(ValueRef&& other);
  ~ValueRef();

  ValueRef& operator=(const ValueRef& other);
  ValueRef& operator=(ValueRef&& other);
  Value* operator->() const;
  Value& operator*() const;

 public:
  Value* get() const;
  void free();
  bool isNullRef() const;
  usize getRefCount() const;

 private:
  Value* ptr;
};

}  // namespace via

#endif
