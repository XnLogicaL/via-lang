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

struct CallInfo
{
    Value* callee;
    CallFlags flags;
    std::vector<ValueRef> args;
};

using NativeCallback = ValueRef (*)(VirtualMachine* vm, CallInfo& ci);

class Closure final
{
  public:
    Closure(const usize argc, const Instruction* pc)
        : m_argc(argc),
          m_native(false),
          m_bytecode(pc)
    {}

    Closure(const usize argc, const NativeCallback callback)
        : m_argc(argc),
          m_native(true),
          m_callback(callback)
    {}

    template <typename... Args>
        requires(std::is_constructible_v<Closure, Args...>)
    [[nodiscard]] static Closure* construct(VirtualMachine* vm, Args&&... args) noexcept
    {
        return vm->get_allocator().emplace<Closure>(forward<Args>(args)...);
    }

  public:
    auto argc() const noexcept { return m_argc; }
    bool is_native() const noexcept { return m_native; }

    auto* bytecode() const noexcept
    {
        debug::require(m_native);
        return m_bytecode;
    }

    auto callback() const noexcept
    {
        debug::require(!m_native);
        return m_callback;
    }

  private:
    const usize m_argc;
    const bool m_native;
    union {
        const Instruction* m_bytecode;
        const NativeCallback m_callback;
    };
};

} // namespace via
