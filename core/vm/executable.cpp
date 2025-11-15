/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "executable.hpp"
#include <cassert>
#include <iomanip>
#include <iostream>
#include <limits>
#include <unordered_map>
#include "debug.hpp"
#include "diagnostics.hpp"
#include "ir/ir.hpp"
#include "module/manager.hpp"
#include "module/module.hpp"
#include "sema/const.hpp"
#include "sema/types.hpp"
#include "support/ansi.hpp"
#include "support/bit.hpp"
#include "vm/instruction.hpp"

namespace ir = via::ir;

void via::detail::set_null_dst_trap(
    Executable& exe,
    const std::optional<uint16_t>& dst
) noexcept
{
    if (!dst.has_value()) {
        debug::bug("destination register must not be null in this context");
    }
}

template <>
void via::detail::ir_lower_expr<ir::ExprConstant>(
    Executable& exe,
    const ir::ExprConstant* ir_expr_constant,
    std::optional<uint16_t> dst
) noexcept
{
    set_null_dst_trap(exe, dst);

    const ConstValue& cvalue = ir_expr_constant->value;

    switch (cvalue.kind()) {
    case ValueKind::NIL:
        exe.push_instruction(OpCode::LOADNIL, {*dst});
        break;
    case ValueKind::BOOL:
        exe.push_instruction(
            cvalue.value<ValueKind::BOOL>() ? OpCode::LOADTRUE : OpCode::LOADFALSE,
            {*dst}
        );
        break;
    case ValueKind::INT: {
        int64_t integer = cvalue.value<ValueKind::INT>();
        if (integer <= std::numeric_limits<int32_t>::max() &&
            integer >= std::numeric_limits<int32_t>::min()) {
            uint16_t b, c;
            int32_t val32 = static_cast<int32_t>(integer); // preserve sign
            unpack_halves(static_cast<uint32_t>(val32), b, c);
            exe.push_instruction(OpCode::LOADINT, {*dst, b, c});
            break;
        }
        [[fallthrough]];
    }
    default:
        exe.push_constant(cvalue);
        exe.push_instruction(OpCode::LOADK, {*dst, (uint16_t) exe.constant_id()});
        break;
    }
}

template <>
void via::detail::ir_lower_expr<ir::ExprSymbol>(
    Executable& exe,
    const ir::ExprSymbol* ir_expr_symbol,
    std::optional<uint16_t> dst
) noexcept
{
    set_null_dst_trap(exe, dst);

    auto& frame = exe.m_stack.top();
    if (auto lref = frame.get_local(ir_expr_symbol->symbol)) {
        exe.push_instruction(OpCode::GETLOCAL, {*dst, lref->id});
    }
}

template <>
void via::detail::ir_lower_expr<ir::ExprModuleAccess>(
    Executable& exe,
    const ir::ExprModuleAccess* ir_expr_module_access,
    std::optional<uint16_t> dst
) noexcept
{
    set_null_dst_trap(exe, dst);

    exe.push_instruction(
        OpCode::GETIMPORT,
        {
            *dst,
            static_cast<uint16_t>(ir_expr_module_access->mod_id),
            static_cast<uint16_t>(ir_expr_module_access->key_id),
        }
    );
}

template <>
void via::detail::ir_lower_expr<ir::ExprBinary>(
    Executable& exe,
    const ir::ExprBinary* ir_expr_binary,
    std::optional<uint16_t> dst
) noexcept
{
    set_null_dst_trap(exe, dst);

    uint16_t opid = static_cast<uint16_t>(ir_expr_binary->op);
    uint16_t rlhs = exe.m_reg_state.alloc(), rrhs = exe.m_reg_state.alloc();

    exe.lower_expr(ir_expr_binary->lhs, rlhs);
    exe.lower_expr(ir_expr_binary->rhs, rrhs);

    if (opid >= static_cast<uint16_t>(BinaryOp::ADD) &&
        opid <= static_cast<uint16_t>(BinaryOp::MOD)) {
        /* TODO: Check if rhs is constexpr, in which case increment base by one
         * for K instructions*/
        uint16_t base = static_cast<uint16_t>(OpCode::IADD) +
                        static_cast<uint16_t>(ir_expr_binary->op);

        if (ir_expr_binary->lhs->type.unwrap()->is_integral()) {
            if (ir_expr_binary->rhs->type.unwrap()->is_float()) {
                base += 2; // FP mode
                exe.push_instruction(OpCode::TOFLOAT, {rlhs, rlhs});
            }
        } else {
            base += 2; // FP mode

            if (ir_expr_binary->rhs->type.unwrap()->is_integral()) {
                exe.push_instruction(OpCode::TOFLOAT, {rrhs, rrhs});
            }
        }

        exe.push_instruction(static_cast<OpCode>(base), {*dst, rlhs, rrhs});
    } else if (opid >= static_cast<uint16_t>(BinaryOp::AND) &&
               opid <= static_cast<uint16_t>(BinaryOp::OR)) {
        /* TODO: Check if rhs is constexpr, in which case increment base by one
         * for K instructions*/
        uint16_t base = static_cast<uint16_t>(OpCode::AND) +
                        static_cast<uint16_t>(ir_expr_binary->op);
        exe.push_instruction(static_cast<OpCode>(base), {*dst, rlhs, rrhs});
    } else if (opid >= static_cast<uint16_t>(BinaryOp::BAND) &&
               opid <= static_cast<uint16_t>(BinaryOp::BSHR)) {
        /* TODO: Check if rhs is constexpr, in which case increment base by one
         * for K instructions*/
        uint16_t base = static_cast<uint16_t>(OpCode::BAND) +
                        static_cast<uint16_t>(ir_expr_binary->op);
        exe.push_instruction(static_cast<OpCode>(base), {*dst, rlhs, rrhs});
    }

    exe.push_instruction(OpCode::FREE2, {rlhs, rrhs});
    exe.m_reg_state.free_all(rlhs, rrhs);
}

template <>
void via::detail::ir_lower_expr<ir::ExprCall>(
    Executable& exe,
    const ir::ExprCall* ir_expr_call,
    std::optional<uint16_t> dst
) noexcept
{
    uint16_t callee = exe.m_reg_state.alloc();
    auto args = ir_expr_call->args;
    std::reverse(args.begin(), args.end());

    for (const auto& arg: args) {
        exe.lower_expr(arg, callee);
        exe.push_instruction(OpCode::PUSH, {callee});
    }

    exe.lower_expr(ir_expr_call->callee, callee);
    exe.push_instruction(OpCode::CALL, {callee});
    exe.push_instruction(OpCode::FREE1, {callee});
    exe.m_reg_state.free(callee);

    if (dst.has_value()) {
        exe.push_instruction(OpCode::GETTOP, {*dst});
    }
}

template <>
void via::detail::ir_lower_expr<ir::ExprCast>(
    Executable& exe,
    const ir::ExprCast* ir_expr_cast,
    std::optional<uint16_t> dst
) noexcept
{
    using enum BuiltinKind;

    set_null_dst_trap(exe, dst);
    exe.lower_expr(ir_expr_cast->expr, dst);

    if (ir_expr_cast->cast == ir_expr_cast->expr->type) {
        // Redundant cast
        return;
    }

    auto& type_ctx = exe.m_module->manager().type_context();

    if TRY_COERCE (const BuiltinType, cast_bultin_type, ir_expr_cast->cast.unwrap()) {
        if TRY_COERCE (const BuiltinType,
                       expr_builtin_type,
                       ir_expr_cast->expr->type.unwrap()) {
            static std::unordered_map<const Type*, OpCode> cast_rules = {
                {BuiltinType::instance(type_ctx, BuiltinKind::INT), OpCode::TOINT},
                {BuiltinType::instance(type_ctx, BuiltinKind::FLOAT), OpCode::TOFLOAT},
                {BuiltinType::instance(type_ctx, BuiltinKind::BOOL), OpCode::TOBOOL},
                {BuiltinType::instance(type_ctx, BuiltinKind::STRING), OpCode::TOSTRING},
            };

            if (auto it = cast_rules.find(cast_bultin_type); it != cast_rules.end()) {
                exe.push_instruction(it->second, {*dst, *dst});
            } else {
                debug::bug("unmapped builtin cast directive");
            }
        }
    }
}

void via::Executable::lower_expr(
    const ir::Expr* expr,
    std::optional<uint16_t> dst
) noexcept
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

    debug::unimplemented(std::format("lower_expr({}, uint16_t)", VIA_TYPENAME(*expr)));
#undef VISIT_EXPR
}

template <>
void via::detail::ir_lower_stmt<ir::StmtVarDecl>(
    Executable& exe,
    const ir::StmtVarDecl* ir_stmt_var_decl
) noexcept
{
    auto dst = exe.m_reg_state.alloc();
    exe.lower_expr(ir_stmt_var_decl->expr, dst);
    exe.push_instruction(OpCode::PUSH, {dst});
    exe.push_instruction(OpCode::FREE1, {dst});
    exe.m_reg_state.free(dst);

    auto& frame = exe.m_stack.top();
    frame.set_local(ir_stmt_var_decl->symbol);
}

template <>
void via::detail::ir_lower_stmt<ir::StmtInstruction>(
    Executable& exe,
    const ir::StmtInstruction* ir_stmt_instr
) noexcept
{
    exe.m_bytecode.push_back(ir_stmt_instr->instr);
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
    if (ir_stmt_block->term != nullptr) {
        exe.lower_term(ir_stmt_block->term);
    }
}

template <>
void via::detail::ir_lower_stmt<ir::StmtFuncDecl>(
    Executable& exe,
    const ir::StmtFuncDecl* ir_stmt_func_decl
) noexcept
{
    auto dst = exe.m_reg_state.alloc();
    auto pc = exe.push_instruction(OpCode::NOP);
    exe.lower_stmt(ir_stmt_func_decl->body);

    size_t offset = exe.program_counter() - pc + 1;
    uint16_t high, low;

    unpack_halves(static_cast<uint32_t>(offset), high, low);

    exe.push_instruction(OpCode::PUSH, {dst});
    exe.push_instruction(OpCode::FREE1, {dst});
    exe.set_instruction(pc, OpCode::NEWCLOSURE, {dst, high, low});
    exe.m_reg_state.free(dst);
}

template <>
void via::detail::ir_lower_stmt<ir::StmtExpr>(
    Executable& exe,
    const ir::StmtExpr* ir_stmt_expr
) noexcept
{
    exe.lower_expr(ir_stmt_expr->expr, std::nullopt);
}

void via::Executable::lower_stmt(const ir::Stmt* stmt) noexcept
{
#define VISIT_STMT(TYPE)                                                                 \
    if TRY_COERCE (const TYPE, _INNER, stmt)                                             \
        return detail::ir_lower_stmt<TYPE>(*this, _INNER);

    VISIT_STMT(ir::StmtVarDecl)
    VISIT_STMT(ir::StmtFuncDecl)
    VISIT_STMT(ir::StmtInstruction)
    VISIT_STMT(ir::StmtBlock)
    VISIT_STMT(ir::StmtExpr)

    debug::unimplemented(std::format("lower_stmt({})", VIA_TYPENAME(*stmt)));
#undef VISIT_STMT
}

template <>
void via::detail::ir_lower_term<ir::TrReturn>(
    Executable& exe,
    const ir::TrReturn* ir_term_ret
) noexcept
{
    if (ir_term_ret->val) {
        uint16_t reg = exe.m_reg_state.alloc();
        exe.lower_expr(ir_term_ret->val, reg);
        exe.push_instruction(OpCode::RET, {reg});
        exe.m_reg_state.free(reg);
    } else {
        exe.push_instruction(OpCode::RETNIL);
    }
}

template <>
void via::detail::ir_lower_term<ir::TrBranch>(
    Executable& exe,
    const ir::TrBranch* ir_term_branch
) noexcept
{
    uint16_t high, low;
    unpack_halves(ir_term_branch->target->id, high, low);
    exe.push_instruction(OpCode::JMP, {high, low});
}

template <>
void via::detail::ir_lower_term<ir::TrCondBranch>(
    Executable& exe,
    const ir::TrCondBranch* ir_term_cond_branch
) noexcept
{
    uint16_t thigh, tlow, fhigh, flow;
    unpack_halves(ir_term_cond_branch->iftrue->id, thigh, tlow);
    unpack_halves(ir_term_cond_branch->iffalse->id, fhigh, flow);

    uint16_t reg = exe.m_reg_state.alloc();
    exe.lower_expr(ir_term_cond_branch->cnd, reg);
    exe.push_instruction(OpCode::JMPIF, {reg, thigh, tlow});
    exe.push_instruction(OpCode::JMP, {fhigh, flow});
    exe.m_reg_state.free(reg);
}

void via::Executable::lower_term(const ir::Term* term) noexcept
{
#define VISIT_TERM(TYPE)                                                                 \
    if TRY_COERCE (const TYPE, _INNER, term)                                             \
        return detail::ir_lower_term<TYPE>(*this, _INNER);

    VISIT_TERM(ir::TrReturn);
    VISIT_TERM(ir::TrBranch);
    VISIT_TERM(ir::TrCondBranch);
    VISIT_TERM(ir::TrContinue);
    VISIT_TERM(ir::TrBreak);

    debug::unimplemented(std::format("lower_term({})", VIA_TYPENAME(*term)));
#undef VISIT_TERM
}

via::Executable* via::Executable::build_from_ir(
    Module* module,
    DiagContext& diags,
    const IRTree& ir_tree,
    ExeFlags flags
) noexcept
{
    auto& alloc = module->allocator();
    auto* exe = alloc.emplace<Executable>(diags);
    exe->m_module = module;
    exe->m_flags = flags;

    for (const auto& stmt: ir_tree) {
        exe->lower_stmt(stmt);
    }

    exe->lower_jumps();
    exe->push_instruction(OpCode::HALT);
    return exe;
}

void via::Executable::lower_jumps() noexcept
{
    size_t pc = 0;
    for (Instruction& instr: m_bytecode) {
        switch (instr.op) {
        case OpCode::JMP: {
            uint32_t label = pack_halves<uint32_t>(instr.a, instr.b);
            uint32_t address = m_labels.at(label);

            uint32_t offset;
            if (address < pc) {
                instr.op = OpCode::JMPBACK;
                offset = pc - address - 1; // store positive distance
            } else {
                offset = address - pc + 1;
            }

            unpack_halves(offset, instr.a, instr.b);
            break;
        }
        case OpCode::JMPIF:
        case OpCode::JMPIFX: {
            uint32_t label = pack_halves<uint32_t>(instr.b, instr.c);
            uint32_t address = m_labels.at(label);

            uint32_t offset;
            if (address < pc) {
                instr.op =
                    (instr.op == OpCode::JMPIF) ? OpCode::JMPBACKIF : OpCode::JMPBACKIFX;
                offset = pc - address - 1; // positive distance
            } else {
                offset = address - pc + 1;
            }

            unpack_halves(offset, instr.b, instr.c);
            break;
        }
        default:
            break;
        }
        ++pc;
    }
}

std::string via::Executable::to_string() const
{
    std::ostringstream oss;
    oss << ansi::format(
        "[disassembly of program code]:\n",
        ansi::Foreground::YELLOW,
        ansi::Background::NONE,
        ansi::Style::UNDERLINE
    );

    oss << ansi::format(
        "  pc      opcode           operands\n"
        "  ------  ---------------  ---------------\n",
        ansi::Foreground::NONE,
        ansi::Background::NONE,
        ansi::Style::FAINT
    );

    for (size_t pc = 0; const Instruction& insn: m_bytecode) {
        oss << "  "
            << ansi::format(
                   std::format("0x{:0>4x}", pc * 8),
                   ansi::Foreground::NONE,
                   ansi::Background::NONE,
                   ansi::Style::FAINT
               );
        oss << "  " << insn.to_string(true, pc) << "\n";
        pc++;
    }
    oss << ansi::format(
        "\n[disassembly of program data]:\n",
        ansi::Foreground::YELLOW,
        ansi::Background::NONE,
        ansi::Style::UNDERLINE
    );

    oss << ansi::format(
        "  id      type        data\n"
        "  ------  ----------  ---------------\n",
        ansi::Foreground::NONE,
        ansi::Background::NONE,
        ansi::Style::FAINT
    );

    for (size_t i = 0; const auto& cv: m_constants) {
        oss << "  "
            << ansi::format(
                   std::format("0x{:0>4x}", i++),
                   ansi::Foreground::NONE,
                   ansi::Background::NONE,
                   ansi::Style::FAINT
               );
        oss << "  " << std::left << std::setw(21)
            << ansi::format(
                   std::string(via::to_string(cv.kind())),
                   ansi::Foreground::MAGENTA,
                   ansi::Background::NONE,
                   ansi::Style::BOLD
               );
        oss << "  " << ansi::format(std::string(cv.to_string()), ansi::Foreground::GREEN);
        oss << "\n";
    }
    return oss.str();
}
