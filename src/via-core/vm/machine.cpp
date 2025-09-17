/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "machine.h"
#include "value.h"
#include "value_ref.h"

via::ValueRef via::VirtualMachine::push_local(ValueRef val)
{
    m_stack.push(reinterpret_cast<uptr>(val.get()));
    return get_local(m_stack.size() - 1);
}

void via::VirtualMachine::set_local(usize sp, ValueRef val)
{
    *m_stack.at(sp) = reinterpret_cast<uptr>(val.get());
}

via::ValueRef via::VirtualMachine::get_local(usize sp)
{
    return ValueRef(this, reinterpret_cast<Value*>(m_stack.at(sp)));
}

via::ValueRef via::VirtualMachine::get_constant(u16 id)
{
    auto cv = m_exe->constants().at(id);
    auto* val = Value::construct(this, cv);
    return ValueRef(this, val);
}
