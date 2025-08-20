// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "stack.h"
#include <ranges>

namespace via {

namespace sema {

Optional<LocalRef> Frame::get_local(StringView symbol) {
  for (i64 i = locals.size() - 1; i >= 0; --i) {
    Local& local = locals[i];
    if (local.get_symbol() == symbol) {
      return LocalRef{static_cast<u16>(i), local};
    }
  }

  return nullopt;
}

void Frame::set_local(StringView symbol,
                      const ast::LValue* lval,
                      const ast::ExprNode* rval,
                      const ast::TypeNode* type,
                      u64 quals) {
  usize version = 0;

  if (auto lref = get_local(symbol)) {
    version = lref->local.get_version() + 1;
  }

  locals.emplace_back(symbol, lval, rval, type, version, quals);
}

}  // namespace sema

}  // namespace via
