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
    bool b;
    char* str;
  } u;

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

  constexpr char* as_cstring(HeapAllocator& alloc) const {
    switch (kind) {
      case String:
        return alloc.strdup(u.str);
      case Nil:
        return alloc.strdup("nil");
      case Bool:
        return alloc.strdup(u.b ? "true" : "false");
      case Int:
        return alloc.strdup(std::to_string(u.i).c_str());
      case Float:
        return alloc.strdup(std::to_string(u.fp).c_str());
      default:
        break;
    }

    VIA_BUG("bad kind value");
  }
};

}  // namespace core

}  // namespace via

#endif
