// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_CONDITIONAL_H_
#define VIA_CORE_CONDITIONAL_H_

#include <via/config.h>

namespace via
{

template <typename... Cds>
constexpr bool all(Cds... bs)
{
  return (bs && ...);
}

template <typename... Cds>
constexpr bool any(Cds... bs)
{
  return (bs || ...);
}

template <typename... Cds>
constexpr bool none(Cds... bs)
{
  return (!bs && ...);
}

}  // namespace via

#endif
