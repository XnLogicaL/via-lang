// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_LOCATION_H_
#define VIA_CORE_LOCATION_H_

#include <via/config.h>
#include <via/types.h>

namespace via
{

struct RelLoc
{
  usize line;
  usize offset;
};

struct SourceLoc
{
  usize begin;
  usize end;

  // Returns the absolute location as a relative location.
  RelLoc to_relative(const String& source) const;
};

}  // namespace via

#endif
