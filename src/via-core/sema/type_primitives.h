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

template <usize Sz>
using type_name = Array<char, Sz>;

template <usize Sz>
consteval auto make_type_name(const char (&str)[Sz]) {
  type_name<Sz> result;

  for (usize i = 0; i < Sz; ++i)
    result[i] = str[i];

  return result;
}

template <usize N1, usize N2>
consteval auto concat_type_name(const char (&a)[N1], const char (&b)[N2]) {
  type_name<N1 + N2 - 1> result{};  // -1 because both have '\0'

  for (usize i = 0; i < N1 - 1; ++i)
    result[i] = a[i];

  for (usize i = 0; i < N2; ++i)
    result[i + N1 - 1] = b[i];

  return result;
}

struct nil_type {};
struct bool_type {};
struct int_type {};
struct float_type {};
struct string_type {};

template <type T>
struct array_type {
  using generics = pack<T>;
};

template <typename T>
struct type_info {
  static constexpr bool is_primitive = false;
  static constexpr auto name = make_type_name("<non-primitive>");
};

template <>
struct type_info<nil_type> {
  static constexpr bool is_primitive = true;
  static constexpr auto name = make_type_name("nil");
};

template <>
struct type_info<bool_type> {
  static constexpr bool is_primitive = true;
  static constexpr auto name = make_type_name("boolean");
};

template <>
struct type_info<int_type> {
  static constexpr bool is_primitive = true;
  static constexpr auto name = make_type_name("int");
};

template <>
struct type_info<float_type> {
  static constexpr bool is_primitive = true;
  static constexpr auto name = make_type_name("float");
};

template <>
struct type_info<string_type> {
  static constexpr bool is_primitive = true;
  static constexpr auto name = make_type_name("string");
};

template <type T>
struct type_info<array_type<T>> {
  static constexpr bool is_primitive = true;
  static constexpr auto name =
      concat_type_name(concat_type_name("array<", type_info<T>::name), ">");
};

template <typename T>
inline constexpr auto is_primitive_v = type_info<T>::is_primitive;

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
