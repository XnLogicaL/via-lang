/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include "debug.h"
#include "executable.h"
#include "instruction.h"
#include "module/symbol.h"
#include "stack.h"

namespace via {
namespace config {
namespace vm {

VIA_CONSTANT size_t REGISTER_COUNT = std::numeric_limits<u16>::max() + 1;

}
} // namespace config

class VirtualMachine;

namespace detail {

template <bool SingleStep, bool OverridePC>
void execute_impl(VirtualMachine* vm);

}

enum class CallFlags : u8
{
    NONE = 0,
    PROTECT = 1 << 0,
    ALL = 0xFF,
};

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

class ValueRef;
class ModuleManager;
class VirtualMachine final
{
  public:
    template <bool, bool>
    friend void detail::execute_impl(VirtualMachine*);
    friend class Snapshot;

  public:
    VirtualMachine(Module* module, const Executable* exe)
        : m_exe(exe),
          m_module(module),
          m_alloc(),
          m_bp(exe->bytecode().data()),
          m_pc(m_bp),
          m_stack(m_alloc),
          m_registers(std::make_unique<Value*[]>(config::vm::REGISTER_COUNT))
    {
        debug::require(!exe->bytecode().empty(), "illformed header");
    }

  public:
    Stack<uintptr_t>& get_stack() { return m_stack; }
    ScopedAllocator& get_allocator() { return m_alloc; }
    ValueRef get_import(SymbolId module_id, SymbolId key_id);
    ValueRef get_constant(u16 id);
    void push_local(ValueRef val);
    ValueRef get_local(size_t sp);
    void call(ValueRef callee, CallFlags flags = CallFlags::NONE);
    void return_(ValueRef value);
    void execute();
    void execute_once();

  protected:
    const Executable* m_exe;
    Module* m_module;
    ScopedAllocator m_alloc;
    uintptr_t* m_sp;           // Saved stack pointer
    uintptr_t* m_fp = nullptr; // Frame pointer
    const Instruction* m_bp;   // Program base pointer
    const Instruction* m_pc;   // Program counter
    Stack<uintptr_t> m_stack;
    std::unique_ptr<Value*[]> m_registers;
};

} // namespace via
