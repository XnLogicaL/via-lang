// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "stack.h"
#include <ranges>

namespace via {

namespace sema {

Optional<LocalRef> Frame::get_local(String symbol) {
  const auto pred = [this, &symbol](const Local& local) {
    return local.get_symbol() == symbol &&
           local.get_version() == vtable[symbol];
  };

  if (auto it = std::ranges::find_if(locals, pred); it != locals.end()) {
    u16 id = std::ranges::distance(locals.begin(), it);
    return LocalRef{id, *it};
  }

  return nullopt;
}

void Frame::set_local(String symbol,
                      const ast::LValue* lval,
                      const ast::ExprNode* rval,
                      const ast::TypeNode* type,
                      u64 quals) {
  usize version = vtable[symbol];
  vtable[symbol] = version + 1;
  locals.emplace_back(symbol, lval, rval, type, version, quals);
}

void Frame::m_eliminate_dead_vtable() {
  for (auto it = vtable.begin(); it != vtable.end();) {
    if (!get_local(it->first)) {
      it = vtable.erase(it);
    } else {
      ++it;
    }
  }
}

}  // namespace sema

}  // namespace via
