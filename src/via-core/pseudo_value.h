// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_PSEUDO_VALUE_H_
#define VIA_CORE_PSEUDO_VALUE_H_

#include <via/config.h>
#include "vm/value.h"

namespace via {

namespace core {

struct PseudoValue {
  using int_type = vm::Value::int_type;
  using float_type = vm::Value::float_type;
  using bool_type = vm::Value::bool_type;

  enum Kind {
    Nil,
    Int,
    Float,
    Bool,
    String,
  } kind;

  union Un {
    int_type i;
    float_type fp;
    bool_type b;
    char* str;
  } u;

  constexpr Optional<int_type> as_cint() const;
  constexpr Optional<float_type> as_cfloat() const;
  constexpr bool_type as_cbool() const;
  constexpr char* as_cstring(HeapAllocator& alloc) const;
};

}  // namespace core

}  // namespace via

#endif
