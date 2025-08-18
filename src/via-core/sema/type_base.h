// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_BASE_H_
#define VIA_CORE_SEMA_TYPE_BASE_H_

#include <via/config.h>
#include <via/types.h>

namespace via {

namespace sema {

namespace types {

template <typename T>
struct is_type {
  static constexpr bool value = false;
};

template <typename T>
inline constexpr auto is_type_v = is_type<T>::value;

template <typename T>
concept type = is_type_v<T>;

}  // namespace types

}  // namespace sema

}  // namespace via

#endif
