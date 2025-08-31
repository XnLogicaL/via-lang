// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "stack.h"
#include "debug.h"

namespace via
{

namespace sema
{

Optional<LocalRef> Frame::get_local(StringView symbol)
{
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
                      const ast::Expr* rval,
                      const ast::Type* type,
                      u64 quals)
{
  usize version;
  if (auto lref = get_local(symbol)) {
    version = lref->local.get_version() + 1;
  } else {
    version = 0;
  }

  locals.emplace_back(symbol, lval, rval, type, version, quals);
}

namespace stack
{

static Vec<Frame> stack{Frame()};

void reset()
{
  stack = Vec<Frame>{Frame()};
}

void push(Frame&& frame)
{
  stack.push_back(std::move(frame));
}

usize size()
{
  return stack.size();
}

Frame& top()
{
  debug::assertm(!stack.empty());
  return stack.back();
}

Frame* at(usize pos)
{
  if (pos > size()) {
    return nullptr;
  } else {
    return &stack[pos];
  }
}

}  // namespace stack

}  // namespace sema

}  // namespace via
