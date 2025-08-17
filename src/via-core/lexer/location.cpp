// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "location.h"

namespace via {

Location AbsLocation::to_relative(const FileBuf& source) const {
  usize line = 1;
  usize init = 0;

  for (usize i = 0; i < begin; ++i) {
    if (source[i] == '\n') {
      ++line;
      init = i + 1;
    }
  }

  usize column = begin - init;
  return {line, column};
}

}  // namespace via
