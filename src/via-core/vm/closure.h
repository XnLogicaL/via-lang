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
#include "instruction.h"
#include "machine.h"

namespace via {

class Closure final
{
  public:
    Closure(const usize argc, const Instruction* pc) :
        m_argc(argc),
        m_bytecode(pc)
    {}

    static Closure* construct(VirtualMachine* vm, const usize argc, const Instruction* pc) noexcept
    {
        return vm->get_allocator().emplace<Closure>(argc, pc);
    }

  public:
    auto argc() const noexcept { return m_argc; }
    auto* bytecode() const noexcept { return m_bytecode; }

  private:
    const usize m_argc;
    const Instruction* m_bytecode;
};

} // namespace via
