// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_UTILS_H_
#define VIA_CORE_SEMA_TYPE_UTILS_H_

#include <via/config.h>
#include <via/types.h>

namespace via {

namespace sema {

namespace types {

template <typename, typename = void>
struct has_type_member : std::false_type {};

template <typename T>
struct has_type_member<T, std::void_t<typename T::type>> : std::true_type {};

template <typename T>
struct is_valid_type : has_type_member<T> {};

template <typename T>
inline constexpr auto is_valid_type_v = is_valid_type<T>::value;

}  // namespace types

}  // namespace sema

}  // namespace via

#endif
