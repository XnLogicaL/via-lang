/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "machine.h"
#include "debug.h"
#include "module/defs.h"
#include "module/manager.h"
#include "module/module.h"
#include "spdlog/spdlog.h"
#include "value.h"
#include "value_ref.h"
#include "vm/closure.h"

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

via::ValueRef via::VirtualMachine::get_import(SymbolId module_id, SymbolId key_id)
{
    ModuleManager* manager = m_module->get_manager();
    if (auto module = manager->get_module_by_name(module_id)) {
        if (auto def = module->lookup(key_id)) {
            if TRY_COERCE (const FunctionDef, fn_def, *def) {
                size_t argc = fn_def->parms.size();
                Closure* closure;

                if (fn_def->kind == ImplKind::NATIVE) {
                    closure = Closure::create(this, argc, fn_def->code.native);
                }
                else {
                    goto error;
                }

                return ValueRef(this, Value::create(this, closure));
            }
        }
    }

// TODO: Better error handling
error:
    debug::bug("invalid call to VirtualMachine::get_import");
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
    // Keep callee alive just in case
    callee->m_rc++;

    Closure* cl = callee->function_value();

    m_stack.push((uintptr_t) callee.get());            // Save callee pointer
    m_stack.push((uintptr_t) flags);                   // Save flags
    m_stack.push((uintptr_t) m_pc + !cl->is_native()); // Save return PC
    m_stack.push((uintptr_t) m_fp);                    // Save old FP

    m_fp = &m_stack.top();

    if (cl->is_native()) {
        CallInfo ci;
        ci.callee = callee.get();
        ci.flags = flags;

        uintptr_t* top = &m_stack.top();
        for (uintptr_t* ptr = top; ptr > top - (ptrdiff_t) cl->get_argc(); --ptr) {
            Value* arg = reinterpret_cast<Value*>(*ptr);
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

    uintptr_t* top = &m_stack.top();
    for (uintptr_t* sp = top; sp > m_fp; --sp) {
        uintptr_t raw = *sp;
        if (raw) {
            Value* val = reinterpret_cast<Value*>(raw);
            val->unref();
        }
    }

    m_stack.jump(m_fp + 1);

    m_fp = reinterpret_cast<uintptr_t*>(m_stack.pop()); // old FP (as a pointer value)
    m_pc = reinterpret_cast<const Instruction*>(m_stack.pop()); // saved PC

    auto flags = static_cast<CallFlags>(m_stack.pop()); // saved flags
    auto* callee = reinterpret_cast<Value*>(m_stack.pop());
    callee->unref();

    // Push return value
    push_local(value.is_null() ? ValueRef(this, Value::create(this)) : value);
}
