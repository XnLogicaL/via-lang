// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "stack.h"
#include <ranges>

namespace via {

namespace sema {

Optional<LocalRef> Frame::find_local(String symbol) {
  const auto pred = [&symbol](const Local& local) {
    return local.get_symbol() == symbol;
  };

  if (auto it = std::ranges::find_if(locals, pred); it != locals.end()) {
    u16 id = std::ranges::distance(locals.begin(), it);
    return LocalRef{id, *it};
  }

  return nullopt;
}

}  // namespace sema

}  // namespace via
