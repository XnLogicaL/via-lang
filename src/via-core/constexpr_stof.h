// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_CONSTEXPR_STOF_H_
#define VIA_CORE_CONSTEXPR_STOF_H_

#include <via/config.h>
#include <via/types.h>

namespace via {

inline constexpr bool is_digit(char c) {
  return c >= '0' && c <= '9';
}

// Based on:
// https://stackoverflow.com/questions/25195176/how-do-i-convert-a-c-string-to-a-int-at-compile-time
template <typename T>
  requires std::is_floating_point_v<T>
constexpr Optional<T> stof(StringView str, usize* pos = NULL) {
  size_t i = 0;
  const size_t n = str.size();

  while (i < n && (str[i] == ' ' || str[i] == '\t'))
    ++i;

  if (i == n)
    return nullopt;

  int sign = 1;
  if (str[i] == '+') {
    ++i;
  } else if (str[i] == '-') {
    sign = -1;
    ++i;
  }

  T int_part = 0;
  bool int_found = false;
  while (i < n && is_digit(str[i])) {
    int_found = true;
    int_part = int_part * 10 + (str[i] - '0');
    ++i;
  }

  // Parse fractional part
  T frac_part = 0;
  T frac_div = 1;
  if (i < n && str[i] == '.') {
    ++i;
    bool frac_found = false;
    while (i < n && is_digit(str[i])) {
      frac_found = true;
      frac_part = frac_part * 10 + (str[i] - '0');
      frac_div *= 10;
      ++i;
    }
    if (!frac_found && !int_found)
      return nullopt;  // "." or "-." is invalid
  } else if (!int_found) {
    // No digits at all
    return nullopt;
  }

  T value = int_part + frac_part / frac_div;

  // Parse exponent part
  if (i < n && (str[i] == 'e' || str[i] == 'E')) {
    ++i;
    if (i == n)
      return nullopt;

    int exp_sign = 1;
    if (str[i] == '+') {
      ++i;
    } else if (str[i] == '-') {
      exp_sign = -1;
      ++i;
    }

    if (i == n || !is_digit(str[i]))
      return nullopt;

    int exponent = 0;
    while (i < n && is_digit(str[i])) {
      exponent = exponent * 10 + (str[i] - '0');
      ++i;
    }

    T exp_val = 1;
    for (int j = 0; j < exponent; ++j)
      exp_val *= 10;

    if (exp_sign < 0)
      value /= exp_val;
    else
      value *= exp_val;
  }

  if (pos)
    *pos = i;

  return sign * value;
}

}  // namespace via

#endif
