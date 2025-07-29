// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_LOCATION_H_
#define VIA_CORE_LOCATION_H_

#include <via/config.h>
#include <util/buffer.h>

namespace via {

namespace core {

using FileBuf = Buffer<char>;

namespace lex {

struct Location {
  usize line;
  usize offset;
};

struct AbsLocation {
  usize begin;
  usize end;

  // Returns the absolute location as a relative location.
  Location to_relative(const FileBuf& source) const;
};

} // namespace lex

} // namespace core

} // namespace via

#endif
