// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_FORMAT_VEC_H
#define VIA_HAS_HEADER_FORMAT_VEC_H

#include "common.h"

namespace via::utils {

template<typename T>
using fmt_vector_t = const std::vector<T>&;

template<typename T>
using fmt_func_t = const std::function<std::string(const T&)>&;

template<typename T>
std::string format_vector(
  fmt_vector_t<T> vec = {},
  fmt_func_t<T> to_str = {},
  char delimiter_begin = '{',
  char delimiter_end = '}'
) {
  std::ostringstream oss;
  oss << delimiter_begin;

  size_t index = 0;
  for (const T& elem : vec) {
    oss << to_str(elem);

    if (index++ < vec.size() - 1) {
      oss << ", ";
    }
  }

  oss << delimiter_end;
  return oss.str();
}

} // namespace via::utils

#endif
