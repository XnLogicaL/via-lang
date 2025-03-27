// =========================================================================================== |
// This file is a part of The via Programming Language and is licensed under GNU GPL v3.0      |
// =========================================================================================== |

#ifndef _VIA_FORMAT_VEC_H
#define _VIA_FORMAT_VEC_H

#include "common.h"

VIA_NAMESPACE_UTIL_BEGIN

template<typename T>
using FmtVector = const std::vector<T>&;

template<typename T>
using FmtFunction = const std::function<std::string(const T&)>&;

template<typename T>
std::string format_vector(
  FmtVector<T>   vec             = {},
  FmtFunction<T> to_str          = {},
  char           delimiter_begin = '{',
  char           delimiter_end   = '}'
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

VIA_NAMESPACE_END

#endif
