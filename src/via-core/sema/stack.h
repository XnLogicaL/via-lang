// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_STACK_H_
#define VIA_CORE_SEMA_STACK_H_

#include <via/config.h>
#include <via/types.h>
#include "local.h"

namespace via
{

namespace sema
{

class Frame final
{
 public:
  Local& top() { return mLocals.back(); }

  Optional<LocalRef> getLocal(StringView symbol);
  void setLocal(StringView symbol,
                const ast::LValue* lval,
                const ast::Expr* rval,
                const ast::Type* type,
                u64 quals = 0ULL);

  void save() { mStkPtr = mLocals.size(); }
  void restore() { mLocals.resize(mStkPtr); }

 private:
  usize mStkPtr;
  Vec<Local> mLocals;
};

namespace stack
{

void reset();
void push(Frame&& frame);
usize size();
Frame& top();
Frame* at(usize pos);

}  // namespace stack

}  // namespace sema

}  // namespace via

#endif
