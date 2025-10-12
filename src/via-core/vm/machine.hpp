/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cstddef>
#include <functional>
#include <iostream>
#include <ostream>
#include <string>
#include <via/config.hpp>
#include "debug.hpp"
#include "executable.hpp"
#include "instruction.hpp"
#include "module/symbol.hpp"
#include "stack.hpp"
#include "support/utility.hpp"

namespace via {
namespace config {
namespace vm {

VIA_CONSTANT size_t REGISTER_COUNT = std::numeric_limits<uint16_t>::max() + 1;

}
} // namespace config

class Closure;
class ValueRef;
class VirtualMachine;

#define FOR_EACH_INTERRUPT(X)                                                            \
    X(NONE)                                                                              \
    X(ERROR)

enum class Interrupt : uint8_t
{
    FOR_EACH_INTERRUPT(DEFINE_ENUM)
};

DEFINE_TO_STRING(Interrupt, FOR_EACH_INTERRUPT(DEFINE_CASE_TO_STRING));

enum class IntAction
{
    RESUME,
    REINTERP,
    EXIT,
};

using InterruptHook = void (*)(VirtualMachine*, Interrupt, void*);

enum class CallFlags : uint8_t
{
    NONE = 0,
    PROTECT = 1 << 0,
    ALL = 0xFF,
};

namespace detail {

template <bool SingleStep, bool OverridePC>
void execute_impl(VirtualMachine* vm);

template <Interrupt Int>
IntAction handle_interrupt_impl(VirtualMachine* vm);

} // namespace detail

class Value;
class Snapshot
{
  public:
    explicit Snapshot(VirtualMachine* vm) noexcept;

  public:
    std::string to_string() const noexcept;

  public:
    const uintptr_t stack_ptr;
    const uintptr_t frame_ptr;
    const Instruction* program_counter;
    const size_t rel_program_counter;
    const std::vector<uintptr_t> stack;
    const std::vector<Value*> registers;
};

struct ErrorInt
{
    std::string msg;
    std::ostream* out;
    const uintptr_t* fp;
    const Instruction* pc;
};

class ValueRef;
class ModuleManager;
class VirtualMachine final
{
  public:
    template <bool, bool>
    friend void detail::execute_impl(VirtualMachine*);

    template <Interrupt>
    friend IntAction detail::handle_interrupt_impl(VirtualMachine*);

    friend class Snapshot;
    friend class Debugger;

  public:
    VirtualMachine(Module* module, const Executable* exe)
        : m_exe(exe),
          m_alloc(),
          m_module(module),
          m_bp(exe->bytecode().data()),
          m_pc(m_bp),
          m_stack(m_alloc),
          m_registers(std::make_unique<Value*[]>(config::vm::REGISTER_COUNT))
    {
        debug::require(!exe->bytecode().empty(), "illformed header");
    }

  public:
    Stack<uintptr_t>& get_stack() { return m_stack; }
    ScopedAllocator& allocator() { return m_alloc; }
    ValueRef get_import(SymbolId module_id, SymbolId key_id);
    ValueRef get_constant(uint16_t id);

    void set_int_hook(InterruptHook hook) { m_int_hook = hook; }
    void set_interrupt(Interrupt code, void* arg = nullptr) noexcept
    {
        if (m_int_arg != nullptr) {
            m_alloc.free(m_int_arg);
        }
        m_int = code;
        m_int_arg = arg;
    }

    void push_local(ValueRef val);
    ValueRef get_local(size_t sp);
    void call(ValueRef callee, CallFlags flags = CallFlags::NONE);
    void return_(ValueRef value);
    void raise(std::string msg, std::ostream* out = &std::cerr);
    void execute();
    void execute_once();

  protected:
    bool has_interrupt() const { return m_int != Interrupt::NONE; }
    IntAction handle_interrupt();
    Closure* unwind_stack(std::function<bool(
                              const uintptr_t* fp,
                              const Instruction* pc,
                              const CallFlags flags,
                              ValueRef callee
                          )> pred);

  protected:
    const Executable* m_exe;
    ScopedAllocator m_alloc;
    Module* m_module;
    uintptr_t* m_sp;           // Saved stack pointer
    uintptr_t* m_fp = nullptr; // Frame pointer
    const Instruction* m_bp;   // Program base pointer
    const Instruction* m_pc;   // Program counter
    Interrupt m_int = Interrupt::NONE;
    InterruptHook m_int_hook = nullptr;
    void* m_int_arg;
    Stack<uintptr_t> m_stack;
    std::unique_ptr<Value*[]> m_registers;
};

} // namespace via
