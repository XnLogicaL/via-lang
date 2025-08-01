// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SHARED_VISIT_AS_H_
#define VIA_SHARED_VISIT_AS_H_

#include <via/config.h>

namespace via {

template <typename T, typename... Ts, typename Fn>
bool visit_as(T* ptr, Fn&& fn) {
  return (... || try_cast_and_call<Ts>(ptr, fn));
}

template <typename T, typename U, typename Fn>
bool try_cast_and_call(T* ptr, Fn&& fn) {
  if (auto* derived = dynamic_cast<U*>(ptr)) {
    fn(*derived);  // T is deduced here
    return true;
  }
  return false;
}

}  // namespace via

#endif
