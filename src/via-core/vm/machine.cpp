/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "machine.hpp"
#include <ostream>
#include "closure.hpp"
#include "debug.hpp"
#include "instruction.hpp"
#include "module/defs.hpp"
#include "module/manager.hpp"
#include "module/module.hpp"
#include "ref.hpp"
#include "value.hpp"

using Vk = via::ValueKind;

template <>
via::IntAction via::detail::handle_interrupt_impl<via::Interrupt::NONE>(VirtualMachine* vm
)
{
    debug::bug("attempt to handle interrupt NONE");
}

template <>
via::IntAction
via::detail::handle_interrupt_impl<via::Interrupt::ERROR>(VirtualMachine* vm)
{
    Closure* handler = vm->unwind_stack([&](auto, auto, auto flags, auto) {
        return flags & CallFlags::PROTECT;
    });

    if (handler == nullptr) {
        auto* error = reinterpret_cast<ErrorInt*>(vm->m_int_arg);
        (*error->out) << error->msg;

        return IntAction::EXIT;
    }
    return IntAction::RESUME;
}

via::Snapshot::Snapshot(VirtualMachine* vm) noexcept
    : stack_ptr(vm->m_sp - vm->m_stack.base()),
      frame_ptr(vm->m_fp ? vm->m_fp - vm->m_stack.base() : 0),
      program_counter(vm->m_pc),
      rel_program_counter(vm->m_pc - vm->m_bp),
      stack(vm->m_stack.begin(), vm->m_stack.end()),
      registers(vm->m_registers.get(), vm->m_registers.get() + config::vm::REGISTER_COUNT)
{}

std::string via::Snapshot::to_string() const noexcept
{
    std::ostringstream oss;
    return oss.str();
}

via::IntAction via::VirtualMachine::handle_interrupt()
{
#define DEFINE_INTERRUPT_HANDLER(INT)                                                    \
    case Interrupt::INT:                                                                 \
        return detail::handle_interrupt_impl<Interrupt::INT>(this);

    if (m_int_hook != nullptr) {
        m_int_hook(this, m_int, m_int_arg);
    }

    switch (m_int) {
        FOR_EACH_INTERRUPT(DEFINE_INTERRUPT_HANDLER)
    default:
        break;
    }

    return IntAction::RESUME;
#undef DEFINE_INTERRUPT_HANDLER
}

via::Closure* via::VirtualMachine::unwind_stack(std::function<bool(
                                                    const uintptr_t* fp,
                                                    const Instruction* pc,
                                                    const CallFlags flags,
                                                    ValueRef callee
                                                )> pred)
{
    for (uintptr_t* fp = m_fp; fp != nullptr;) {
        m_stack.jump(fp + 1);

        auto* this_fp = reinterpret_cast<uintptr_t*>(m_stack.pop());
        auto* this_pc = reinterpret_cast<const Instruction*>(m_stack.pop());
        auto flags = static_cast<CallFlags>(m_stack.pop());
        auto* callee = reinterpret_cast<Value*>(m_stack.pop());

        if (pred(this_fp, this_pc, flags, ValueRef(this, callee))) {
            return callee->function_value();
        } else {
            fp = this_fp;
            callee->unref();
        }
    }
    return nullptr;
}

via::ValueRef via::VirtualMachine::get_import(SymbolId module_id, SymbolId key_id)
{
    auto& manager = m_module->manager();

    if (auto module = manager.get_module_by_name(module_id)) {
        if (auto def = module->lookup(key_id)) {
            if TRY_COERCE (const FunctionDef, fn_def, *def) {
                size_t argc = fn_def->parms.size();
                Closure* closure;

                if (fn_def->kind == ImplKind::NATIVE) {
                    closure = Closure::create(this, argc, fn_def->code.native);
                } else {
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
    val->m_rc++; // Manually increment reference count as the stack is managed manually
    m_stack.push((uintptr_t) val.get()); // Push the value onto the stack
}

via::ValueRef via::VirtualMachine::get_local(size_t sp)
{
    // Ensure the stack pointer is within bounds
    debug::require(sp < m_stack.size(), "invalid stack pointer");
    return ValueRef(this, (Value*) m_stack.at(sp));
}

via::ValueRef via::VirtualMachine::get_constant(uint16_t id)
{
    auto cv = m_exe->constants().at(id); // Get the constant value
    auto* val = Value::create(this, cv); // Create a new value from the constant
    return ValueRef(this, val);
}

void via::VirtualMachine::call(ValueRef callee, CallFlags flags)
{
    callee->m_rc++; // Keep callee alive just in case

    // Get the closure from the callee value
    Closure* cl = callee->function_value();
    uintptr_t* base = &m_stack.top();

    m_stack.push((uintptr_t) callee.get());            // Save callee pointer
    m_stack.push((uintptr_t) flags);                   // Save flags
    m_stack.push((uintptr_t) m_pc + !cl->is_native()); // Save return PC
    m_stack.push((uintptr_t) m_fp);                    // Save old FP

    // Set the new frame pointer
    m_fp = &m_stack.top();

    if (cl->is_native()) {
        CallInfo ci; // Initialize the CallInfo structure
        ci.callee = callee.get();
        ci.flags = flags; // Propagate flags

        // Iterate over the arguments in reverse order
        for (uintptr_t* ptr = base; ptr > base - (ptrdiff_t) cl->get_argc(); --ptr) {
            // Get the argument value
            Value* arg = reinterpret_cast<Value*>(*ptr);
            // Push it onto the CallInfo object
            ci.args.push_back(ValueRef(this, arg));
        }

        // Call the native function
        auto result = cl->get_callback()(this, ci);
        return_(result); // Return the result of the native function call
    } else {
        // Call the non-native function by setting the program counter
        m_pc = cl->get_bytecode();
    }
}

void via::VirtualMachine::return_(ValueRef value)
{
    // Check if the frame pointer is valid
    debug::require(m_fp != nullptr);

    // Get the top of the stack
    uintptr_t* top = &m_stack.top();

    // Iterate over locals inside the stack frame
    for (uintptr_t* local = top; local > m_fp; --local) {
        // Check if the local is not null
        if (*local) {
            // Unreference the local value
            auto* val = reinterpret_cast<Value*>(local);
            val->unref();
        }
    }

    // Jump stack to frame pointer
    m_stack.jump(m_fp + 1);

    m_fp = reinterpret_cast<uintptr_t*>(m_stack.pop()); // Old FP (as a pointer value)
    m_pc = reinterpret_cast<const Instruction*>(m_stack.pop()); // Saved PC

    auto flags = static_cast<CallFlags>(m_stack.pop());     // Saved flags
    auto* callee = reinterpret_cast<Value*>(m_stack.pop()); // Callee
    callee->unref();                                        // Unreference the callee

    // Push return value
    push_local(value.is_null() ? ValueRef(this, Value::create(this)) : value);
}

void via::VirtualMachine::raise(std::string msg, std::ostream* out)
{
    auto* error = m_alloc.emplace<ErrorInt>();
    error->msg = msg;
    error->out = out;
    error->fp = m_fp;
    error->pc = m_pc;
    set_interrupt(Interrupt::ERROR, error);
}
