// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_DEBUG_H_
#define VIA_CORE_DEBUG_H_

#include <via/config.h>
#include <via/types.h>
#include <source_location>

namespace via
{

namespace debug
{

void assertm(bool cond,
             std::string message = "<no-message-specified>",
             std::source_location __loc = std::source_location::current());

[[noreturn]] void bug(
    std::string what,
    std::source_location __loc = std::source_location::current());

[[noreturn]] void todo(
    std::string what,
    std::source_location __loc = std::source_location::current());

[[noreturn]] void unimplemented(
    std::string what = "<no-message-specified>",
    std::source_location __loc = std::source_location::current());

template <typename T, char LDel = '{', char RDel = '}'>
std::string dump(const Vec<T>& vec,
                 Function<std::string(const std::remove_cv_t<T>&)> fn)
{
  std::ostringstream oss;
  oss << LDel;

  for (usize i = 0; i < vec.size(); i++) {
    oss << fn(vec[i]);
    if (i != vec.size() - 1) {
      oss << ", ";
    }
  }

  oss << RDel;
  return oss.str();
}

}  // namespace debug

}  // namespace via

#endif
