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

template <typelist = type_list<>>
struct nil_type {
  using type = nil_type;
  using generics = type_list<>;
};

template <typelist = type_list<>>
struct bool_type {
  using type = bool_type;
  using generics = type_list<>;
};

template <typelist = type_list<>>
struct int_type {
  using type = int_type;
  using generics = type_list<>;
};

template <typelist = type_list<>>
struct float_type {
  using type = float_type;
  using generics = type_list<>;
};

template <typelist = type_list<>>
struct string_type {
  using type = string_type;
  using generics = type_list<>;
};

template <typelist Ts>
struct array_type {
  using type = array_type<Ts>;
  using generics = Ts;
};

template <typelist Ts>
struct dict_type {
  using type = dict_type<Ts>;
  using generics = Ts;
};

template <template <typelist> typename T, typelist Ts = type_list<>>
inline constexpr auto is_primitive_v = std::is_same_v<T<Ts>, nil_type<Ts>> ||
                                       std::is_same_v<T<Ts>, bool_type<Ts>> ||
                                       std::is_same_v<T<Ts>, int_type<Ts>> ||
                                       std::is_same_v<T<Ts>, float_type<Ts>> ||
                                       std::is_same_v<T<Ts>, string_type<Ts>> ||
                                       std::is_same_v<T<Ts>, array_type<Ts>> ||
                                       std::is_same_v<T<Ts>, dict_type<Ts>>;

template <template <typelist> typename T, typelist Ts>
  requires is_primitive_v<T, Ts>
struct is_type<T<Ts>> {
  static constexpr bool value = true;
};

template <>
struct type_info<nil_type<>> {
  static constexpr bool valid = true;
  static constexpr auto name = make_type_name("nil");
};

template <>
struct type_info<bool_type<>> {
  static constexpr bool valid = true;
  static constexpr auto name = make_type_name("boolean");
};

template <>
struct type_info<int_type<>> {
  static constexpr bool valid = true;
  static constexpr auto name = make_type_name("int");
};

template <>
struct type_info<float_type<>> {
  static constexpr bool valid = true;
  static constexpr auto name = make_type_name("float");
};

template <>
struct type_info<string_type<>> {
  static constexpr bool valid = true;
  static constexpr auto name = make_type_name("string");
};

template <typelist Ts>
struct type_info<array_type<Ts>> {
  static constexpr bool valid = true;
  static constexpr auto name = concat_type_name(
      concat_type_name("array<", type_info<type_at_t<0, Ts>>::name),
      ">");  // "array<" + {T} + ">"
};

template <>
struct type_info<array_type<type_list<>>> {
  static constexpr bool valid = false;
  static constexpr auto name = make_type_name("array<>");
};

template <typelist Ts>
struct type_info<dict_type<Ts>> {
  static constexpr bool valid = true;
  static constexpr auto name = concat_type_name(
      concat_type_name(
          concat_type_name("dict<", type_info<type_at_t<0, Ts>>::name),
          concat_type_name(", ", type_info<type_at<1, Ts>>::name)),
      ">");  // "dict<" + {T0} + ", " + {T1} + ">"
};

template <>
struct type_info<dict_type<type_list<>>> {
  static constexpr bool valid = false;
  static constexpr auto name = make_type_name("dict<>");
};

template <type T>
struct type_info<dict_type<type_list<T>>> {
  static constexpr bool valid = false;
  static constexpr auto name = make_type_name("dict<>");
};

}  // namespace types

}  // namespace sema

}  // namespace via

#endif
