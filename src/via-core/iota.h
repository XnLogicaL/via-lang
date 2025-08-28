// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_IOTA_H_
#define VIA_CORE_IOTA_H_

#include <via/config.h>
#include <via/types.h>

namespace via
{

template <std::integral T = int>
constexpr T iota() noexcept
{
  static T old(0);
  return old++;
}

}  // namespace via

#endif
