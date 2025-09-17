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

class Value;
class Snapshot
{
  public:
    const uptr stack_ptr;
    const uptr frame_ptr;
    const Instruction program_counter;
    const std::unique_ptr<uptr[]> stack;
    const std::unique_ptr<Value*[]> registers;
};

class ValueRef;
class VirtualMachine final
{
  public:
    // Internal executor
    template <bool, bool>
    friend void execute_impl(VirtualMachine*);

  public:
    VirtualMachine(const Executable* exe) :
        m_exe(exe),
        m_pc(exe->bytecode().data()),
        m_alloc(),
        m_stack(m_alloc),
        m_registers(std::make_unique<Value*[]>(config::vm::REGISTER_COUNT))
    {
        debug::require(!exe->bytecode().empty(), "illformed header");
    }

  public:
    Stack<uptr>& get_stack() { return m_stack; }
    Allocator& get_allocator() { return m_alloc; }
    ValueRef get_constant(u16 id);
    ValueRef push_local(ValueRef val);
    ValueRef get_local(usize sp);
    void set_local(usize sp, ValueRef val);
    void call(ValueRef callee);
    void execute();
    void execute_one();
    Snapshot create_snapshot();

  protected:
    const Executable* m_exe;
    Allocator m_alloc;
    uptr* m_sp;       // saved stack pointer
    const uptr* m_fp; // frame pointer
    const Instruction* m_pc;
    Stack<uptr> m_stack;
    std::unique_ptr<Value*[]> m_registers;
};

} // namespace via
