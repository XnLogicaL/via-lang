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
#include <vector>
#include <via/config.h>
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
    Closure(size_t argc, const Instruction* pc)
        : m_native(false),
          m_argc(argc),
          m_bytecode(pc)
    {}

    Closure(size_t argc, const NativeCallback callback)
        : m_native(true),
          m_argc(argc),
          m_callback(callback)
    {}

    [[nodiscard]] static Closure*
    create(VirtualMachine* vm, size_t argc, const Instruction* pc) noexcept
    {
        return vm->allocator().emplace<Closure>(argc, pc);
    }

    [[nodiscard]] static Closure*
    create(VirtualMachine* vm, size_t argc, const NativeCallback callback) noexcept
    {
        return vm->allocator().emplace<Closure>(argc, callback);
    }

  public:
    auto get_argc() const noexcept { return m_argc; }
    bool is_native() const noexcept { return m_native; }
    auto& get_upvalues() const noexcept { return m_upvs; }
    auto* get_bytecode() const noexcept { return m_bytecode; }
    auto get_callback() const noexcept { return m_callback; }

  private:
    const bool m_native;
    const size_t m_argc;
    std::vector<Value*> m_upvs;

    union {
        const Instruction* m_bytecode;
        const NativeCallback m_callback;
    };
};

} // namespace via
