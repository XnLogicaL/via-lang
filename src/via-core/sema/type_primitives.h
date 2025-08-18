// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_PRIMITIVES_H_
#define VIA_CORE_SEMA_TYPE_PRIMITIVES_H_

#include <via/config.h>
#include <via/types.h>
#include "type_base.h"

namespace via {

namespace sema {

namespace types {

struct nil_type {};
struct bool_type {};
struct int_type {};
struct float_type {};
struct string_type {};

template <typename T>
struct is_primitive {
  static constexpr bool value = false;
  static constexpr auto name = "<non-primitive>"sv;
};

template <>
struct is_primitive<nil_type> {
  static constexpr bool value = true;
  static constexpr auto name = "nil"sv;
};

template <>
struct is_primitive<bool_type> {
  static constexpr bool value = true;
  static constexpr auto name = "boolean"sv;
};

template <>
struct is_primitive<int_type> {
  static constexpr bool value = true;
  static constexpr auto name = "int"sv;
};

template <>
struct is_primitive<float_type> {
  static constexpr bool value = true;
  static constexpr auto name = "float"sv;
};

template <>
struct is_primitive<string_type> {
  static constexpr bool value = true;
  static constexpr auto name = "string"sv;
};

template <typename T>
inline constexpr auto is_primitive_v = is_primitive<T>::value;

template <typename T>
concept primitive = is_primitive_v<T>;

template <primitive T>
struct is_type<T> {
  static constexpr bool value = true;
};

}  // namespace types

}  // namespace sema

}  // namespace via

#endif
