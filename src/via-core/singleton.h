// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SINGLETON_H_
#define VIA_CORE_SINGLETON_H_

#include <via/config.h>
#include <via/types.h>

namespace via {

template <typename T>
  requires(std::is_default_constructible_v<T>)
constexpr T* get_instance() noexcept {
  static T instance;
  return &instance;
}

}  // namespace via

#endif
