/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "option.h"

namespace via
{

// Taken from (slightly modified):
// https://stackoverflow.com/questions/25195176/how-do-i-convert-a-c-string-to-a-int-at-compile-time
template <std::integral T>
constexpr Option<T> stoi(std::string_view str, usize* pos = nullptr)
{
  using namespace std::literals;
  const auto digits = "0123456789abcdefABCDEF"sv;
  const usize begin = str.find_first_of(digits);
  if (begin == std::string_view::npos)
    return nullopt;

  int sign = 1;
  if (begin >= 1 && str[begin - 1U] == '-') {
    sign = -1;
  }

  str.remove_prefix(begin);

  int base = 10;
  if (str.starts_with("0x") || str.starts_with("0X")) {
    base = 16;
    str.remove_prefix(2);
  } else if (str.starts_with("0b") || str.starts_with("0B")) {
    base = 2;
    str.remove_prefix(2);
  }

  usize end = 0;
  while (end < str.size()) {
    char c = str[end];
    if ((base == 2 && (c != '0' && c != '1')) ||
        (base == 10 && !std::isdigit(c)) || (base == 16 && !std::isxdigit(c)))
      break;
    ++end;
  }

  if (end == 0) {
    return nullopt;
  }

  const std::string_view digits_to_parse = str.substr(0, end);

  T result = 0;
  for (usize i = 0; i < digits_to_parse.size(); ++i) {
    char c = digits_to_parse[i];
    T digit = 0;

    if (c >= '0' && c <= '9')
      digit = c - '0';
    else if (c >= 'a' && c <= 'f')
      digit = 10 + (c - 'a');
    else if (c >= 'A' && c <= 'F')
      digit = 10 + (c - 'A');

    if (digit >= base)
      return nullopt;  // invalid digit for base

    result = result * base + digit;
  }

  if (pos != nullptr)
    *pos = begin +
           (str.starts_with("0x") || str.starts_with("0X") ||
                str.starts_with("0b") || str.starts_with("0B")
              ? 2
              : 0) +
           end;

  return result * sign;
}

}  // namespace via
