// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_SHARED_CONSTEXPR_STOI_H_
#define VIA_SHARED_CONSTEXPR_STOI_H_

#include <via/config.h>

namespace via {

// Taken from (slightly modified):
// https://stackoverflow.com/questions/25195176/how-do-i-convert-a-c-string-to-a-int-at-compile-time
template <typename T>
  requires std::is_integral_v<T>
constexpr Optional<T> stoi(StringView str, usize* pos = NULL) {
  using namespace std::literals;
  const auto numbers = "0123456789"sv;

  const usize begin = str.find_first_of(numbers);
  if (begin == StringView::npos)
    return nullopt;

  const auto sign = begin != 0U && str[begin - 1U] == '-' ? -1 : 1;
  str.remove_prefix(begin);

  const auto end = str.find_first_not_of(numbers);
  if (end != StringView::npos)
    str.remove_suffix(std::size(str) - end);

  auto result = 0;
  auto multiplier = 1U;
  for (ptrdiff_t i = std::size(str) - 1U; i >= 0; --i) {
    auto number = str[i] - '0';
    result += number * multiplier * sign;
    multiplier *= 10U;
  }

  if (pos != NULL)
    *pos = begin + std::size(str);

  return result;
}

}  // namespace via

#endif
