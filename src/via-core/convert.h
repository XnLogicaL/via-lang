// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_CONVERT_H_
#define VIA_CORE_CONVERT_H_

#include <fmt/ranges.h>
#include <via/config.h>
#include <via/types.h>
#include <magic_enum/magic_enum.hpp>
#include <ranges>
#include <type_traits>
#include "buffer.h"

namespace via {

template <typename T>
struct Convert {
  static String to_string(const T& t) {
    return Convert<std::decay_t<T>>::to_string(t);
  }
};

template <typename T>
  requires std::is_enum_v<T>
struct Convert<T> {
  static String to_string(const T& t) {
    return String(magic_enum::enum_name(t));
  }
};

template <typename T>
struct Convert<Vec<T>> {
  static String to_string(const Vec<T>& v) {
    auto transform = std::views::transform(
        [](const T& t) { return Convert<T>::to_string(t); });
    return fmt::format("{}", fmt::join(v | transform, ", "));
  }
};

template <typename T>
struct Convert<Buffer<T>> {
  static String to_string(const Buffer<T>& buf) {
    auto range = std::views::counted(buf.begin(), buf.size());
    auto transform = std::views::transform(
        [](const T& t) { return Convert<T>::to_string(t); });
    return fmt::format("{}", fmt::join(range | transform, "\n"));
  }
};

}  // namespace via

template <typename T>
struct fmt::formatter<T, char, std::true_type> {
  constexpr auto parse(fmt::format_parse_context& ctx) { return ctx.begin(); }

  template <typename FormatContext>
  auto format(const T& t, FormatContext& ctx) const {
    return fmt::format_to(ctx.out(), "{}", via::Convert<T>::to_string(t));
  }
};

#endif
