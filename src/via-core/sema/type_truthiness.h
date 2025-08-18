// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_TRUTHINESS_H_
#define VIA_CORE_SEMA_TYPE_TRUTHINESS_H_

#include <via/config.h>
#include <via/types.h>
#include "type_base.h"
#include "type_primitives.h"

namespace via {

namespace sema {

namespace types {

template <type T>
struct is_truthy {
  static constexpr bool value = true;
};

template <>
struct is_truthy<nil_type> {
  static constexpr bool value = false;
};

// Yes... booleans are NOT truthy, as they are one of two types that can ever be
// falsy.
template <>
struct is_truthy<bool_type> {
  static constexpr bool value = false;
};

template <type T>
inline constexpr auto is_truthy_v = is_type<T>::value;

}  // namespace types

}  // namespace sema

}  // namespace via

#endif
