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
#include <iostream>
#include <limits>
#include <optional>
#include <via/config.hpp>
#include "diagnostics.hpp"
#include "instruction.hpp"
#include "ir/ir.hpp"
#include "sema/const.hpp"
#include "sema/local_bc.hpp"
#include "sema/register.hpp"
#include "sema/stack.hpp"
#include "support/traits.hpp"

namespace via {
namespace config {

VIA_CONSTANT uint32_t MAGIC = 0x2E766961; // .via

}

class Executable;

namespace detail {

void set_null_dst_trap(Executable& exe, const std::optional<uint16_t>& dst) noexcept;

template <derived_from<ir::Expr> Expr>
void ir_lower_expr(Executable& exe, const Expr* expr, std::optional<uint16_t> dst)
    noexcept
{
    debug::todo(std::format("lower_expr<{}>()", VIA_TYPENAME(Expr)));
}

template <derived_from<ir::Stmt> Stmt>
void ir_lower_stmt(Executable& exe, const Stmt* stmt) noexcept
{
    debug::todo(std::format("lower_stmt<{}>()", VIA_TYPENAME(Stmt)));
}

template <derived_from<ir::Term> Term>
void ir_lower_term(Executable& exe, const Term* term) noexcept
{
    debug::todo(std::format("lower_term<{}>()", VIA_TYPENAME(Term)));
}

} // namespace detail

enum ExeFlags : uint64_t
{
    NONE = 0,
};

class Module;
class Executable final
{
  public:
    friend void
    detail::set_null_dst_trap(Executable&, const std::optional<uint16_t>& dst) noexcept;

    template <derived_from<ir::Expr> Expr>
    friend void
    detail::ir_lower_expr(Executable&, const Expr*, std::optional<uint16_t>) noexcept;

    template <derived_from<ir::Stmt> Stmt>
    friend void detail::ir_lower_stmt(Executable&, const Stmt*) noexcept;

    template <derived_from<ir::Term> Term>
    friend void detail::ir_lower_term(Executable&, const Term*) noexcept;

  public:
    Executable(DiagContext& diags)
        : m_reg_state(diags)
    {
        m_stack.emplace();
    }

    static Executable* build_from_ir(
        Module* module,
        DiagContext& diags,
        const IRTree& ir_tree,
        ExeFlags flags = ExeFlags::NONE
    ) noexcept;

    static Executable* build_from_binary(
        Module* module,
        DiagContext& diags,
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
        if (m_constants.size() >= std::numeric_limits<uint16_t>::max()) {
        }
        m_constants.push_back(std::move(cv));
    }

    void push_instruction(OpCode op, std::array<uint16_t, 3>&& ops = {}) noexcept
    {
        m_bytecode.emplace_back(op, ops[0], ops[1], ops[2]);
    }

    void lower_expr(const ir::Expr* expr, std::optional<uint16_t> dst) noexcept;
    void lower_stmt(const ir::Stmt* stmt) noexcept;
    void lower_term(const ir::Term* term) noexcept;
    void lower_jumps() noexcept;

  private:
    Module* m_module;
    ExeFlags m_flags;
    sema::RegisterState m_reg_state;
    sema::StackState<sema::BytecodeLocal> m_stack;
    std::vector<Instruction> m_bytecode;
    std::vector<sema::ConstValue> m_constants;
    std::unordered_map<size_t, size_t> m_labels;
};

} // namespace via
