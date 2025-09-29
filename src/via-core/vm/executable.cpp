/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "executable.h"
#include <functional>
#include <iomanip>
#include <iostream>
#include <unordered_map>
#include "debug.h"
#include "diagnostics.h"
#include "ir/ir.h"
#include "module/manager.h"
#include "module/module.h"
#include "sema/type.h"
#include "sema/type_context.h"
#include "support/ansi.h"
#include "support/math.h"
#include "vm/instruction.h"

namespace ir = via::ir;

void via::detail::set_null_dst_trap(Executable& exe, const std::optional<u16>& dst)
    noexcept
{
    if (!dst.has_value()) {
        debug::bug("destination register must not be null in this context");
    }
}

template <>
void via::detail::ir_lower_expr<ir::ExprConstant>(
    Executable& exe,
    const ir::ExprConstant* ir_expr_constant,
    std::optional<u16> dst
) noexcept
{
    set_null_dst_trap(exe, dst);
    exe.push_constant(ir_expr_constant->value);
    exe.push_instruction(OpCode::LOADK, {*dst, static_cast<u16>(exe.constant_id())});
}

template <>
void via::detail::ir_lower_expr<ir::ExprSymbol>(
    Executable& exe,
    const ir::ExprSymbol* ir_expr_symbol,
    std::optional<u16> dst
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
    std::optional<u16> dst
) noexcept
{
    set_null_dst_trap(exe, dst);

    exe.push_instruction(
        OpCode::GETIMPORT,
        {
            *dst,
            static_cast<u16>(ir_expr_module_access->mod_id),
            static_cast<u16>(ir_expr_module_access->key_id),
        }
    );
}

template <>
void via::detail::ir_lower_expr<ir::ExprBinary>(
    Executable& exe,
    const ir::ExprBinary* ir_expr_binary,
    std::optional<u16> dst
) noexcept
{
    set_null_dst_trap(exe, dst);

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
        } else {
            base += 2; // FP mode

            if (ir_expr_binary->rhs->type->is_integral()) {
                exe.push_instruction(OpCode::ITOF, {rrhs, rrhs});
            }
        }

        exe.push_instruction(static_cast<OpCode>(base), {*dst, rlhs, rrhs});
    } else if (opid >= static_cast<u16>(BinaryOp::AND) &&
               opid <= static_cast<u16>(BinaryOp::OR)) {
        /* TODO: Check if rhs is constexpr, in which case increment base by one
         * for K instructions*/
        u16 base = static_cast<u16>(OpCode::AND) + static_cast<u16>(ir_expr_binary->op);
        exe.push_instruction(static_cast<OpCode>(base), {*dst, rlhs, rrhs});
    } else if (opid >= static_cast<u16>(BinaryOp::BAND) &&
               opid <= static_cast<u16>(BinaryOp::BSHR)) {
        /* TODO: Check if rhs is constexpr, in which case increment base by one
         * for K instructions*/
        u16 base = static_cast<u16>(OpCode::BAND) + static_cast<u16>(ir_expr_binary->op);
        exe.push_instruction(static_cast<OpCode>(base), {*dst, rlhs, rrhs});
    }

    exe.push_instruction(OpCode::FREE2, {rlhs, rrhs});
    exe.m_reg_state.free_all(rlhs, rrhs);
}

template <>
void via::detail::ir_lower_expr<ir::ExprCall>(
    Executable& exe,
    const ir::ExprCall* ir_expr_call,
    std::optional<u16> dst
) noexcept
{
    u16 callee = exe.m_reg_state.alloc();
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

struct BuiltinCastKey
{
    const via::sema::Type *from, *to;
};

template <>
struct std::hash<BuiltinCastKey>
{
    size_t operator()(const BuiltinCastKey& key) const noexcept
    {
        return via::hash_all(via::hash_ptr(key.from), via::hash_ptr(key.to));
    }
};

template <>
struct std::equal_to<BuiltinCastKey>
{
    size_t operator()(const BuiltinCastKey& lhs, const BuiltinCastKey& rhs) const noexcept
    {
        return lhs.from == rhs.from && lhs.to == rhs.to;
    }
};

static std::unordered_map<BuiltinCastKey, via::OpCode>
get_builtin_cast_rules(via::sema::TypeContext& type_ctx) noexcept
{
#define DEFINE_RULE(FROM, TO, OPCODE)                                                    \
    {                                                                                    \
        {.from = type_ctx.get_builtin(via::sema::BuiltinKind::FROM),                     \
         .to = type_ctx.get_builtin(via::sema::BuiltinKind::TO)},                        \
            via::OpCode::OPCODE                                                          \
    }

    return {
        DEFINE_RULE(BOOL, INT, BTOI),
        DEFINE_RULE(FLOAT, INT, FTOI),
        DEFINE_RULE(STRING, INT, STOI),
        DEFINE_RULE(INT, FLOAT, ITOF),
        DEFINE_RULE(BOOL, FLOAT, BTOF),
        DEFINE_RULE(STRING, FLOAT, STOF),
        DEFINE_RULE(INT, BOOL, ITOB),
        DEFINE_RULE(STRING, BOOL, STOB),
        DEFINE_RULE(INT, STRING, ITOS),
        DEFINE_RULE(FLOAT, STRING, FTOS),
        DEFINE_RULE(BOOL, STRING, BTOS),
    };

#undef DEFINE_RULE
}

template <>
void via::detail::ir_lower_expr<ir::ExprCast>(
    Executable& exe,
    const ir::ExprCast* ir_expr_cast,
    std::optional<u16> dst
) noexcept
{
    using enum sema::BuiltinKind;

    set_null_dst_trap(exe, dst);
    exe.lower_expr(ir_expr_cast->expr, dst);

    if TRY_COERCE (const sema::BuiltinType, cast_bultin_type, ir_expr_cast->cast) {
        if TRY_COERCE (const sema::BuiltinType,
                       expr_builtin_type,
                       ir_expr_cast->expr->type) {
            static const auto builtin_cast_rules =
                get_builtin_cast_rules(exe.m_module->get_manager()->get_type_context());

            BuiltinCastKey key{
                .from = expr_builtin_type,
                .to = cast_bultin_type,
            };

            if (auto it = builtin_cast_rules.find(key); it != builtin_cast_rules.end()) {
                exe.push_instruction(it->second, {*dst, *dst});
            } else {
                debug::bug("unmapped builtin cast directive");
            }
        }
    }
}

void via::Executable::lower_expr(const ir::Expr* expr, std::optional<u16> dst) noexcept
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

    debug::unimplemented(std::format("lower_expr({}, u16)", VIA_TYPENAME(*expr)));
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
    auto dst = exe.m_reg_state.alloc();
    exe.push_instruction(OpCode::NEWCLOSURE, {dst});
    exe.lower_stmt(ir_stmt_func_decl->body);
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
    exe.lower_expr(ir_stmt_expr->expr, std::nullopt);
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

    debug::unimplemented(std::format("lower_stmt({})", VIA_TYPENAME(*stmt)));
#undef VISIT_STMT
}

via::Executable* via::Executable::build_from_ir(
    Module* module,
    DiagContext& diags,
    const IRTree& ir_tree,
    ExeFlags flags
) noexcept
{
    auto& alloc = module->get_allocator();
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
    for (size_t pc = 0; Instruction & instr: m_bytecode) {
        auto opid = static_cast<u16>(instr.op);
        if (opid >= static_cast<u16>(OpCode::JMP) &&
            opid <= static_cast<u16>(OpCode::JMPIFX)) {
            auto address = m_labels[instr.a];
            auto offset = static_cast<ssize_t>(address) - static_cast<ssize_t>(pc);

            if (offset < 0) {
                // backward jump → bump opcode to BACK variant
                instr.op = static_cast<OpCode>(opid + 3);
                instr.a = static_cast<u32>(-offset);
            } else {
                // forward jump → keep opcode
                instr.a = static_cast<u32>(offset);
            }
        }
        ++pc;
    }
}

std::string via::Executable::to_string() const
{
    std::ostringstream oss;
    oss << ansi::format(
        "[section .text]\n",
        ansi::Foreground::YELLOW,
        ansi::Background::NONE,
        ansi::Style::UNDERLINE
    );

    for (size_t pc = 0; const Instruction& insn: m_bytecode) {
        if (auto it = std::find_if(
                m_labels.begin(),
                m_labels.end(),
                [&pc](auto&& p) { return p.second == pc; }
            );
            it != m_labels.end())
            oss << " .LB" << it->first << ":\n";
        oss << "  0x";
        oss << std::right << std::setw(4) << std::setfill('0') << std::hex << (pc * 8)
            << std::dec;
        oss << "  " << insn.to_string(true) << "\n";
        pc++;
    }

    oss << ansi::format(
        "[section .data]\n",
        ansi::Foreground::YELLOW,
        ansi::Background::NONE,
        ansi::Style::UNDERLINE
    );

    for (size_t i = 0; const auto& cv: m_constants) {
        oss << "  "
            << ansi::format(
                   "CONSTANT",
                   ansi::Foreground::MAGENTA,
                   ansi::Background::NONE,
                   ansi::Style::BOLD
               );
        oss << " " << i++ << " = " << cv.get_dump() << "\n";
    }

    return oss.str();
}
