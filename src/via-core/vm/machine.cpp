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

using Vk = via::ValueKind;

via::Snapshot::Snapshot(VirtualMachine* vm) noexcept
    : stack_ptr(vm->m_sp - vm->m_stack.base()),
      frame_ptr(vm->m_fp ? vm->m_fp - vm->m_stack.base() : 0),
      program_counter(*vm->m_pc),
      stack(vm->m_stack.begin(), vm->m_stack.end() + 1),
      registers(vm->m_registers.get(), vm->m_registers.get() + config::vm::REGISTER_COUNT)
{}

std::string via::Snapshot::to_string() const noexcept
{
    std::ostringstream oss;
    return oss.str();
}

void via::VirtualMachine::push_local(ValueRef val)
{
    val->m_rc++;
    m_stack.push((uintptr_t) val.get());
}

via::ValueRef via::VirtualMachine::get_local(size_t sp)
{
    return ValueRef(this, (Value*) m_stack.at(sp));
}

via::ValueRef via::VirtualMachine::get_constant(u16 id)
{
    auto cv = m_exe->constants().at(id);
    auto* val = Value::create(this, cv);
    return ValueRef(this, val);
}

void via::VirtualMachine::call(ValueRef callee, CallFlags flags)
{
    callee->m_rc++;

    m_stack.push((uintptr_t) callee.m_ptr); // Save callee
    m_stack.push((uintptr_t) flags);        // Save flags
    m_stack.push((uintptr_t) m_pc);         // Save PC
    m_stack.push((uintptr_t) m_fp);         // Save old FP

    m_fp = &m_stack.top(); // Set new FP

    Closure* cl = callee->function_value();

    if (cl->is_native()) {
        CallInfo ci;
        ci.callee = callee.get();
        ci.flags = flags;

        for (uintptr_t* ptr = &m_stack.top(); ptr > &m_stack.top() - cl->get_argc();
             ptr--) {
            auto* arg = reinterpret_cast<Value*>(*ptr);
            ci.args.push_back(ValueRef(this, arg));
        }

        auto result = cl->get_callback()(this, ci);
        return_(result);
    }
    else {
        m_pc = cl->get_bytecode();
    }
}

void via::VirtualMachine::return_(ValueRef value)
{
    debug::require(m_fp != nullptr);

    for (uintptr_t* sp = &m_stack.top(); sp > m_fp; sp--) {
        auto* val = (Value*) sp;
        val->unref();
    }

    m_stack.jump(m_fp);

    m_fp = (uintptr_t*) m_stack.pop();
    m_pc = (const Instruction*) m_stack.pop();

    u64 flags = m_stack.pop();
    auto* callee = (Value*) m_stack.pop(); // Pop callee pointer
    callee->unref();

    [[likely]] if (!value.is_null()) {
        push_local(value);
    }
}
