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
#include "ir/ir.h"
#include "sema/bytecode_local.h"
#include "sema/const_value.h"
#include "sema/register.h"
#include "sema/stack.h"
#include "support/type_traits.h"

namespace via {
namespace config {

VIA_CONSTANT u32 MAGIC = 0x2E766961; // .via

}

class Executable;

namespace detail {

template <derived_from<ir::Expr> Expr>
void ir_lower_expr(Executable& exe, const Expr* expr, u16 dst) noexcept
{
    debug::todo(std::format("lower_expr<{}>()", VIA_TYPENAME(Expr)));
}

template <derived_from<ir::Stmt> Stmt>
void ir_lower_stmt(Executable& exe, const Stmt* stmt) noexcept
{
    debug::todo(std::format("lower_stmt<{}>()", VIA_TYPENAME(Stmt)));
}

} // namespace detail

enum ExeFlags : u64
{
    NONE = 0,
};

class Module;
class Executable final
{
  public:
    template <derived_from<ir::Expr> Expr>
    friend void detail::ir_lower_expr(Executable&, const Expr*, u16) noexcept;

    template <derived_from<ir::Stmt> Stmt>
    friend void detail::ir_lower_stmt(Executable&, const Stmt*) noexcept;

  public:
    Executable() { m_stack.emplace(); }

    static Executable*
    build_from_ir(Module* module, const IRTree& ir_tree, ExeFlags flags = ExeFlags::NONE)
        noexcept;

    static Executable* build_from_binary(
        Module* module,
        std::ostream& bytes,
        ExeFlags flags = ExeFlags::NONE
    ) noexcept;

  public:
    auto flags() const noexcept { return m_flags; }
    auto& constants() const noexcept { return m_constants; }
    auto& bytecode() const noexcept { return m_bytecode; }
    std::string to_string() const;

  private:
    size_t program_counter() const noexcept { return m_bytecode.size() - 1; }
    size_t constant_id() const noexcept { return m_constants.size() - 1; }
    size_t set_label(size_t id) noexcept
    {
        m_labels[id] = program_counter();
        return m_labels.size() - 1;
    }

    void push_constant(sema::ConstValue cv) noexcept
    {
        m_constants.push_back(std::move(cv));
    }
    void push_instruction(OpCode op, std::array<u16, 3>&& ops = {}) noexcept
    {
        m_bytecode.emplace_back(op, ops[0], ops[1], ops[2]);
    }

    void lower_expr(const ir::Expr* expr, u16 dst) noexcept;
    void lower_stmt(const ir::Stmt* stmt) noexcept;
    void lower_jumps() noexcept;

  private:
    Module* m_module;
    ExeFlags m_flags;
    u16 m_junk_reg;
    sema::RegisterState m_reg_state;
    sema::StackState<sema::BytecodeLocal> m_stack;
    std::vector<Instruction> m_bytecode;
    std::vector<sema::ConstValue> m_constants;
    std::unordered_map<size_t, size_t> m_labels;
};

} // namespace via
