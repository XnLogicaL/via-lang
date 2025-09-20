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
#include "stack.h"

namespace via {
namespace config {
namespace vm {

CONSTANT usize REGISTER_COUNT = std::numeric_limits<u16>::max() + 1;

}
} // namespace config

class VirtualMachine;

namespace detail {

template <bool SingleStep, bool OverridePC>
void __execute(VirtualMachine* vm);

}

enum CallFlags : u64
{
    CF_NONE = 0,
    CF_PROTECT = 1 << 0,
    CF_ALL = std::numeric_limits<u64>::max(),
};

class Value;
class Snapshot
{
  public:
    explicit Snapshot(VirtualMachine* vm) noexcept;

  public:
    std::string to_string() const noexcept;

  public:
    const uptr stack_ptr;
    const uptr frame_ptr;
    const Instruction program_counter;
    const std::vector<uptr> stack;
    const std::vector<Value*> registers;
};

class ValueRef;
class ModuleManager;
class VirtualMachine final
{
  public:
    template <bool, bool>
    friend void detail::__execute(VirtualMachine*);
    friend class Snapshot;

  public:
    VirtualMachine(Module* module, const Executable* exe)
        : m_exe(exe),
          m_module(module),
          m_pc(exe->bytecode().data()),
          m_alloc(),
          m_stack(m_alloc),
          m_registers(std::make_unique<Value*[]>(config::vm::REGISTER_COUNT))
    {
        debug::require(!exe->bytecode().empty(), "illformed header");
    }

  public:
    Stack<uptr>& get_stack() { return m_stack; }
    ScopedAllocator& get_allocator() { return m_alloc; }
    ValueRef get_constant(u16 id);
    void push_local(ValueRef val);
    ValueRef get_local(usize sp);
    void call(ValueRef callee, CallFlags flags = CF_NONE);
    void return_(ValueRef value);
    void execute();
    void execute_one();

  protected:
    const Executable* m_exe;
    Module* m_module;
    ScopedAllocator m_alloc;
    uptr* m_sp;           // saved stack pointer
    uptr* m_fp = nullptr; // frame pointer
    const Instruction* m_pc;
    Stack<uptr> m_stack;
    std::unique_ptr<Value*[]> m_registers;
};

} // namespace via
