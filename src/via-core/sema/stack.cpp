// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "stack.h"
#include "debug.h"

namespace via
{

namespace sema
{

Option<LocalRef> Frame::getLocal(std::string_view symbol)
{
  for (i64 i = mLocals.size() - 1; i >= 0; --i) {
    Local& local = mLocals[i];
    if (local.getSymbol() == symbol) {
      return LocalRef{static_cast<u16>(i), local};
    }
  }

  return nullopt;
}

void Frame::setLocal(std::string_view symbol,
                     const ast::Expr* lval,
                     const ast::Expr* rval,
                     const ast::Type* type,
                     u64 quals)
{
  usize version;
  if (auto lref = getLocal(symbol)) {
    version = lref->local.getVersion() + 1;
  } else {
    version = 0;
  }

  mLocals.emplace_back(symbol, lval, rval, type, version, quals);
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
