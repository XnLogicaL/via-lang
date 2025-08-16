// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_CONSTEXPR_IPOW_H_
#define VIA_CORE_CONSTEXPR_IPOW_H_

#include <via/config.h>
#include <via/types.h>

namespace via {

template <typename T = int>
  requires std::is_integral_v<T>
constexpr T ipow(T base, T exp) {
  T result = 1;
  for (;;) {
    if (exp & 1)
      result *= base;
    exp >>= 1;
    if (!exp)
      break;
    base *= base;
  }

  return result;
}

}  // namespace via

#endif
