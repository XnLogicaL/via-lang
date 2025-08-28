// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMVER_H_
#define VIA_CORE_SEMVER_H_

#include <via/config.h>
#include <via/types.h>
#include "constexpr_ipow.h"
#include "convert.h"

namespace via
{

struct Version
{
  usize major, minor, patch;
};

template <usize P = 1>
constexpr Version get_semantic_version()
{
  constexpr usize ver = config::version;
  constexpr usize major = ipow<usize>(100, P);
  constexpr usize minor = ipow<usize>(10, P);

  return {
      .major = ver / major,
      .minor = (ver / minor) % minor,
      .patch = ver % minor,
  };
}

template <>
struct Convert<Version>
{
  static String to_string(const Version& v)
  {
    return fmt::format("{}.{}.{}", v.major, v.minor, v.patch);
  }
};

}  // namespace via

#endif
