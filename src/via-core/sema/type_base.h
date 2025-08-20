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

template <type... Ts>
struct type_list {
  static constexpr usize size = sizeof...(Ts);
};

template <typename T>
struct is_type_list {
  static constexpr bool value = false;
};

template <type... Ts>
struct is_type_list<type_list<Ts...>> {
  static constexpr bool value = true;
};

template <typename T>
inline constexpr auto is_type_list_v = is_type_list<T>::value;

template <typename T>
concept typelist = is_type_list_v<T>;

template <usize N, typelist Ls>
struct type_at;

template <usize N, type Head, type... Tail>
struct type_at<N, type_list<Head, Tail...>> {
  using type = typename type_at<N - 1, type_list<Tail...>>::type;
};

template <type Head, type... Tail>
struct type_at<0, type_list<Head, Tail...>> {
  using type = Head;
};

template <usize N, typelist Ls>
using type_at_t = typename type_at<N, Ls>::type;

template <usize N, typelist Ls, type Or>
struct type_at_or {
  using type = std::conditional<N >= Ls::size, type_at_t<N, Ls>, Or>;
};

template <usize N, typelist Ls, type Or>
using type_at_or_t = typename type_at_or<N, Ls, Or>::type;

template <typelist = type_list<>>
struct invalid_type;

template <>
struct is_type<invalid_type<>> {
  static constexpr bool value = true;
};

template <typename T>
struct type_info {
  static constexpr bool valid = false;
  static constexpr auto name = make_type_name("<none-type>");
};

template <>
struct type_info<invalid_type<>> {
  static constexpr bool valid = false;
  static constexpr auto name = make_type_name("<invalid-type>");
};

template <type T>
struct is_valid_type {
  static constexpr bool value = type_info<T>::valid;
};

template <type T>
inline constexpr auto is_valid_type_v = is_valid_type<T>::value;

template <type T, type Or>
struct invalid_or {
  using type = std::conditional_t<is_valid_type_v<T>, T, Or>;
};

template <type T, type Or>
using invalid_or_t = invalid_or<T, Or>::type;

}  // namespace types

}  // namespace sema

}  // namespace via

#endif
