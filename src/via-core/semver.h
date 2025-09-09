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
#include "constexpr_ipow.h"

namespace via
{

struct Version
{
  usize major, minor, patch;
};

template <usize P = 1>
consteval Version getSemanticVersion()
{
  constexpr usize ver = config::kVersion;
  constexpr usize major = ipow<usize>(100, P);
  constexpr usize minor = ipow<usize>(10, P);

  return {
    .major = ver / major,
    .minor = (ver / minor) % minor,
    .patch = ver % minor,
  };
}

inline std::string toString(const Version& v)
{
  return std::format("{}.{}.{}", v.major, v.minor, v.patch);
}

}  // namespace via
