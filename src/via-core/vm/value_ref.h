// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_VM_VALUE_REF_H_
#define VIA_VM_VALUE_REF_H_

#include <via/config.h>
#include <via/types.h>
#include "memory.h"

namespace via {

class Interpreter;
struct Value;

struct ValueRef {
  Value* ptr;

  void free();
  bool is_null() const;
  usize count_refs() const;

  VIA_NOMOVE(ValueRef);
  VIA_IMPLCOPY(ValueRef);

  ValueRef(Interpreter* ctx);
  ValueRef(Interpreter* ctx, Value* ptr);
  ~ValueRef();

  Value* operator->() const;
  Value& operator*() const;
};

}  // namespace via

#endif
