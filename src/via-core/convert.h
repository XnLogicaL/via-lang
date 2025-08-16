// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_CONVERT_H_
#define VIA_CORE_CONVERT_H_

#include <via/config.h>
#include <via/types.h>
#include <magic_enum/magic_enum.hpp>

namespace via {

template <typename T>
struct Convert {
  static String to_string(const T& t) {
    if constexpr (std::is_enum_v<T>)
      return magic_enum::enum_name(t);

    return std::to_string(t);
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
