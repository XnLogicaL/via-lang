/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "interpreter.h"
#include "value.h"
#include "value_ref.h"

via::ValueRef via::Interpreter::pushLocal(ValueRef val)
{
  mStack.push(reinterpret_cast<uptr>(val.get()));
  return getLocal(mStack.size() - 1);
}

void via::Interpreter::setLocal(usize mStkPtr, ValueRef val)
{
  *mStack.at(mStkPtr) = reinterpret_cast<uptr>(val.get());
}

via::ValueRef via::Interpreter::getLocal(usize mStkPtr)
{
  return ValueRef(this, reinterpret_cast<Value*>(mStack.at(mStkPtr)));
}

via::ValueRef via::Interpreter::getConstant(u16 id)
{
  auto cv = mExecutable->constants().at(id);
  auto* val = Value::construct(this, cv);
  return ValueRef(this, val);
}
