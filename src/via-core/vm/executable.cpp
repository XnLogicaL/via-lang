/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "executable.h"
#include <iostream>
#include "module/module.h"
#include "support/ansi.h"

namespace ir = via::ir;

template <>
void via::detail::ir_lower_expr<ir::ExprConstant>(
    Executable& exe,
    const ir::ExprConstant* ir_expr_constant,
    u16 dst
) noexcept
{
    exe.push_constant(ir_expr_constant->value);
    exe.push_instruction(OpCode::LOADK, {dst, static_cast<u16>(exe.constant_id())});
}

template <>
void via::detail::ir_lower_expr<ir::ExprSymbol>(
    Executable& exe,
    const ir::ExprSymbol* ir_expr_symbol,
    u16 dst
) noexcept
{
    auto& frame = exe.m_stack.top();
    if (auto lref = frame.get_local(ir_expr_symbol->symbol)) {
        exe.push_instruction(OpCode::GETLOCAL, {dst, lref->id});
    }
}

template <>
void via::detail::ir_lower_expr<ir::ExprModuleAccess>(
    Executable& exe,
    const ir::ExprModuleAccess* ir_expr_module_access,
    u16 dst
) noexcept
{
    // TEMPORARY
    exe.push_instruction(OpCode::GETIMPORT, {dst});
}

template <>
void via::detail::ir_lower_expr<ir::ExprBinary>(
    Executable& exe,
    const ir::ExprBinary* ir_expr_binary,
    u16 dst
) noexcept
{
    u16 opid = static_cast<u16>(ir_expr_binary->op);
    u16 rlhs = exe.m_reg_state.alloc(), rrhs = exe.m_reg_state.alloc();

    exe.lower_expr(ir_expr_binary->lhs, rlhs);
    exe.lower_expr(ir_expr_binary->rhs, rrhs);

    if (opid >= static_cast<u16>(BinaryOp::ADD) &&
        opid <= static_cast<u16>(BinaryOp::MOD)) {
        /* TODO: Check if rhs is constexpr, in which case increment base by one
         * for K instructions*/
        u16 base = static_cast<u16>(OpCode::IADD) + static_cast<u16>(ir_expr_binary->op);

        if (ir_expr_binary->lhs->type->is_integral()) {
            if (ir_expr_binary->rhs->type->is_float()) {
                base += 2; // FP mode
                exe.push_instruction(OpCode::ITOF, {rlhs, rlhs});
            }
        }
        else {
            base += 2; // FP mode

            if (ir_expr_binary->rhs->type->is_integral()) {
                exe.push_instruction(OpCode::ITOF, {rrhs, rrhs});
            }
        }

        exe.push_instruction(static_cast<OpCode>(base), {dst, rlhs, rrhs});
    }
    else if (opid >= static_cast<u16>(BinaryOp::AND) &&
             opid <= static_cast<u16>(BinaryOp::OR)) {
        /* TODO: Check if rhs is constexpr, in which case increment base by one
         * for K instructions*/
        u16 base = static_cast<u16>(OpCode::AND) + static_cast<u16>(ir_expr_binary->op);
        exe.push_instruction(static_cast<OpCode>(base), {dst, rlhs, rrhs});
    }
    else if (opid >= static_cast<u16>(BinaryOp::BAND) &&
             opid <= static_cast<u16>(BinaryOp::BSHR)) {
        /* TODO: Check if rhs is constexpr, in which case increment base by one
         * for K instructions*/
        u16 base = static_cast<u16>(OpCode::BAND) + static_cast<u16>(ir_expr_binary->op);
        exe.push_instruction(static_cast<OpCode>(base), {dst, rlhs, rrhs});
    }

    exe.push_instruction(OpCode::FREE2, {rlhs, rrhs});
    exe.m_reg_state.free_all(rlhs, rrhs);
}

template <>
void via::detail::ir_lower_expr<ir::ExprCall>(
    Executable& exe,
    const ir::ExprCall* ir_expr_call,
    u16 dst
) noexcept
{
    u16 reg = exe.m_reg_state.alloc();
    auto args = ir_expr_call->args;
    std::reverse(args.begin(), args.end());

    for (const auto& arg: args) {
        exe.lower_expr(arg, reg);
        exe.push_instruction(OpCode::PUSH, {reg});
    }

    exe.lower_expr(ir_expr_call->callee, dst);
    exe.push_instruction(OpCode::CALL, {reg});
    exe.push_instruction(OpCode::FREE1, {reg});
    exe.m_reg_state.free(reg);
}

void via::Executable::lower_expr(const ir::Expr* expr, u16 dst) noexcept
{
#define VISIT_EXPR(TYPE)                                                                 \
    if TRY_COERCE (const TYPE, _INNER, expr)                                             \
        return detail::ir_lower_expr<TYPE>(*this, _INNER, dst);

    VISIT_EXPR(ir::ExprConstant);
    VISIT_EXPR(ir::ExprSymbol);
    VISIT_EXPR(ir::ExprAccess);
    VISIT_EXPR(ir::ExprModuleAccess);
    VISIT_EXPR(ir::ExprUnary);
    VISIT_EXPR(ir::ExprBinary);
    VISIT_EXPR(ir::ExprCall);
    VISIT_EXPR(ir::ExprSubscript);
    VISIT_EXPR(ir::ExprCast);
    VISIT_EXPR(ir::ExprTernary);
    VISIT_EXPR(ir::ExprArray);
    VISIT_EXPR(ir::ExprTuple);
    VISIT_EXPR(ir::ExprLambda);

    debug::unimplemented(std::format("lower_expr({}, u16)", TYPENAME(*expr)));
#undef VISIT_EXPR
}

template <>
void via::detail::ir_lower_stmt<ir::StmtVarDecl>(
    Executable& exe,
    const ir::StmtVarDecl* ir_stmt_var_decl
) noexcept
{
    u16 dst = exe.m_reg_state.alloc();
    exe.lower_expr(ir_stmt_var_decl->expr, dst);
    exe.push_instruction(OpCode::PUSH, {dst});
    exe.push_instruction(OpCode::FREE1, {dst});
    exe.m_reg_state.free(dst);

    auto& frame = exe.m_stack.top();
    frame.set_local(ir_stmt_var_decl->symbol);
}

template <>
void via::detail::ir_lower_stmt<ir::StmtBlock>(
    Executable& exe,
    const ir::StmtBlock* ir_stmt_block
) noexcept
{
    exe.set_label(ir_stmt_block->id);
    for (const auto& stmt: ir_stmt_block->stmts) {
        exe.lower_stmt(stmt);
    }

    /* TODO: Lower terminator */
}

template <>
void via::detail::ir_lower_stmt<ir::StmtFuncDecl>(
    Executable& exe,
    const ir::StmtFuncDecl* ir_stmt_func_decl
) noexcept
{
    u16 dst = exe.m_reg_state.alloc();
    exe.push_instruction(OpCode::NEWCLOSURE, {dst});
    exe.lower_stmt(ir_stmt_func_decl->body);
    exe.push_instruction(OpCode::ENDCLOSURE);
    exe.push_instruction(OpCode::PUSH, {dst});
    exe.push_instruction(OpCode::FREE1, {dst});
    exe.m_reg_state.free(dst);
}

template <>
void via::detail::ir_lower_stmt<ir::StmtExpr>(
    Executable& exe,
    const ir::StmtExpr* ir_stmt_expr
) noexcept
{
    exe.lower_expr(ir_stmt_expr->expr, exe.m_junk_reg);
}

void via::Executable::lower_stmt(const ir::Stmt* stmt) noexcept
{
#define VISIT_STMT(TYPE)                                                                 \
    if TRY_COERCE (const TYPE, _INNER, stmt)                                             \
        return detail::ir_lower_stmt<TYPE>(*this, _INNER);

    VISIT_STMT(ir::StmtVarDecl)
    VISIT_STMT(ir::StmtFuncDecl)
    VISIT_STMT(ir::StmtBlock)
    VISIT_STMT(ir::StmtExpr)

    debug::unimplemented(std::format("lower_stmt({})", TYPENAME(*stmt)));
#undef VISIT_STMT
}

via::Executable*
via::Executable::build_from_ir(Module* module, const IRTree& ir, u64 flags) noexcept
{
    auto& alloc = module->get_allocator();
    auto* exe = alloc.emplace<Executable>();
    exe->m_junk_reg = exe->m_reg_state.alloc();

    for (const auto& stmt: ir) {
        exe->lower_stmt(stmt);
    }

    exe->lower_jumps();
    exe->push_instruction(OpCode::HALT);
    return exe;
}

void via::Executable::lower_jumps() noexcept
{
    for (usize pc = 0; Instruction & instr: m_bytecode) {
        auto opid = static_cast<u16>(instr.op);
        if (opid >= static_cast<u16>(OpCode::JMP) &&
            opid <= static_cast<u16>(OpCode::JMPIFX)) {
            auto address = m_labels[instr.a];
            auto offset = static_cast<isize>(address) - static_cast<isize>(pc);

            if (offset < 0) {
                // backward jump → bump opcode to BACK variant
                instr.op = static_cast<OpCode>(opid + 3);
                instr.a = static_cast<u32>(-offset);
            }
            else {
                // forward jump → keep opcode
                instr.a = static_cast<u32>(offset);
            }
        }
        ++pc;
    }
}

std::string via::Executable::get_dump() const
{
    std::ostringstream oss;
    oss << ansi::format(
        "[section .text]\n",
        ansi::Foreground::Yellow,
        ansi::Background::Black,
        ansi::Style::Underline
    );

    for (usize pc = 0; const Instruction& insn: m_bytecode) {
        if (auto it = std::find_if(
                m_labels.begin(),
                m_labels.end(),
                [&pc](auto&& p) { return p.second == pc; }
            );
            it != m_labels.end())
            oss << " .LB" << it->first << ":\n";
        oss << "  " << insn.get_dump() << "\n";
        pc++;
    }

    oss << ansi::format(
        "[section .data]\n",
        ansi::Foreground::Yellow,
        ansi::Background::Black,
        ansi::Style::Underline
    );

    for (const sema::ConstValue& cv: m_constants) {
        oss << "  " << cv.get_dump() << "\n";
    }

    return oss.str();
}
