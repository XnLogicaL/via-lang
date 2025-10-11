/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "builder.hpp"
#include <cpptrace/basic.hpp>
#include <format>
#include <spdlog/spdlog.h>
#include <vector>
#include "ast/ast.hpp"
#include "debug.hpp"
#include "diagnostics.hpp"
#include "ir/ir.hpp"
#include "module/defs.hpp"
#include "module/symbol.hpp"
#include "sema/control.hpp"
#include "sema/stack.hpp"
#include "sema/type.hpp"
#include "support/math.hpp"
#include "support/traits.hpp"

namespace ir = via::ir;
namespace sema = via::sema;
namespace ast = via::ast;

using Ak = ir::ExprAccess::Kind;
using Btk = sema::BuiltinKind;
using LocalQual = sema::IRLocal::Qual;

#define UNARY_OP_CASE(VALID, RESULT)                                                     \
    {                                                                                    \
        .is_valid = [](const sema::Type* type) -> bool { return VALID; },                \
        .get_result = [](sema::TypeContext* ctx, const sema::Type* type                  \
                      ) -> const sema::Type* { return RESULT; },                         \
    }

struct UnaryOpInfo
{
    std::function<bool(const sema::Type*)> is_valid; // predicate
    std::function<const sema::Type*(sema::TypeContext*, const sema::Type*)>
        get_result; // compute result
};

static const UnaryOpInfo UNARY_OP_TABLE[] = {
    /* UnaryOp::NEG */ UNARY_OP_CASE(type->is_arithmetic(), type),
    /* UnaryOp::NOT */ UNARY_OP_CASE(true, ctx->get_builtin(Btk::BOOL)),
    /* UnaryOp::BNOT */ UNARY_OP_CASE(type->is_integral(), type),
};

#define BINARY_OP_CASE(VALID, RESULT)                                                    \
    {                                                                                    \
        .is_valid = [](const sema::Type* lhs, const sema::Type* rhs) -> bool {           \
            return VALID;                                                                \
        },                                                                               \
        .get_result = [](sema::TypeContext* ctx,                                         \
                         const sema::Type* lhs,                                          \
                         const sema::Type* rhs) -> const sema::Type* { return RESULT; }, \
    }

struct BinaryOpInfo
{
    std::function<bool(const sema::Type*, const sema::Type*)> is_valid; // predicate
    std::function<
        const sema::Type*(sema::TypeContext*, const sema::Type*, const sema::Type*)>
        get_result; // compute result
};

#define BINARY_OP_PROMOTE(LHS, RHS)                                                      \
    ctx->get_builtin((LHS->is_float() || RHS->is_float()) ? Btk::FLOAT : Btk::INT)

static const BinaryOpInfo BINARY_OP_TABLE[] = {
    /* BinaryOp::ADD */
    BINARY_OP_CASE(
        lhs->is_arithmetic() && rhs->is_arithmetic(),
        BINARY_OP_PROMOTE(lhs, rhs)
    ),
    /* BinaryOp::SUB */
    BINARY_OP_CASE(
        lhs->is_arithmetic() && rhs->is_arithmetic(),
        BINARY_OP_PROMOTE(lhs, rhs)
    ),
    /* BinaryOp::MUL */
    BINARY_OP_CASE(
        lhs->is_arithmetic() && rhs->is_arithmetic(),
        BINARY_OP_PROMOTE(lhs, rhs)
    ),
    /* BinaryOp::DIV */
    BINARY_OP_CASE(
        lhs->is_arithmetic() && rhs->is_arithmetic(),
        ctx->get_builtin(Btk::FLOAT)
    ),
    /* BinaryOp::POW */
    BINARY_OP_CASE(
        lhs->is_arithmetic() && rhs->is_arithmetic(),
        BINARY_OP_PROMOTE(lhs, rhs)
    ),
    /* BinaryOp::MOD */
    BINARY_OP_CASE(lhs->is_integral() && rhs->is_integral(), ctx->get_builtin(Btk::INT)),
    /* BinaryOp::AND */ BINARY_OP_CASE(true, ctx->get_builtin(Btk::BOOL)),
    /* BinaryOp::OR */ BINARY_OP_CASE(true, ctx->get_builtin(Btk::BOOL)),
    /* BinaryOp::BAND */
    BINARY_OP_CASE(lhs->is_integral() && rhs->is_integral(), ctx->get_builtin(Btk::INT)),
    /* BinaryOp::BOR */
    BINARY_OP_CASE(lhs->is_integral() && rhs->is_integral(), ctx->get_builtin(Btk::INT)),
    /* BinaryOp::BXOR */
    BINARY_OP_CASE(lhs->is_integral() && rhs->is_integral(), ctx->get_builtin(Btk::INT)),
    /* BinaryOp::BSHL */
    BINARY_OP_CASE(lhs->is_integral() && rhs->is_integral(), ctx->get_builtin(Btk::INT)),
    /* BinaryOp::BSHR */
    BINARY_OP_CASE(lhs->is_integral() && rhs->is_integral(), ctx->get_builtin(Btk::INT)),
};

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprLiteral>(
    IRBuilder& builder,
    const ast::ExprLiteral* ast_expr_literal
) noexcept
{
    using enum TokenKind;
    using enum sema::BuiltinKind;

    sema::BuiltinKind kind;

    switch (ast_expr_literal->tok->kind) {
    case LIT_NIL:
        kind = NIL;
        break;
    case LIT_TRUE:
    case LIT_FALSE:
        kind = BOOL;
        break;
    case LIT_INT:
    case LIT_XINT:
    case LIT_BINT:
        kind = INT;
        break;
    case LIT_FLOAT:
        kind = FLOAT;
        break;
    case LIT_STRING:
        kind = STRING;
        break;
    default:
        debug::bug("invalid literal expression");
    }

    return builder.m_type_ctx.get_builtin(kind);
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprSymbol>(
    IRBuilder& builder,
    const ast::ExprSymbol* ast_expr_symbol
) noexcept
{
    auto& frame = builder.m_stack.top();
    auto symbol = ast_expr_symbol->symbol->to_string();
    auto id = builder.intern_symbol(ast_expr_symbol->symbol->to_string());

    if (auto local = frame.get_local(id)) {
        auto* ir_decl = local->local->get_ir_decl();

        if TRY_COERCE (const ir::StmtVarDecl, var_decl, ir_decl) {
            return var_decl->type;
        } else if TRY_COERCE (const ir::StmtFuncDecl, func_decl, ir_decl) {
            std::vector<const sema::Type*> parms;
            for (const auto& parm: func_decl->parms) {
                parms.push_back(parm.type);
            }

            return builder.m_type_ctx.get_function(func_decl->ret, std::move(parms));
        }
    }
    return nullptr;
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprStaticAccess>(
    IRBuilder& builder,
    const ast::ExprStaticAccess* ast_expr_st_access
) noexcept
{
    if TRY_COERCE (const ast::ExprSymbol, symbol, ast_expr_st_access->root) {
        auto& manager = builder.m_module->manager();
        auto low = builder.intern_symbol(symbol->symbol->to_string());

        if (auto module = manager.get_module_by_name(low)) {
            auto high = builder.intern_symbol(ast_expr_st_access->index->to_string());

            if (auto def = module->lookup(high)) {
                if TRY_COERCE (const FunctionDef, func_def, *def) {
                    std::vector<const sema::Type*> parm_types;

                    for (const auto& parm: func_def->parms) {
                        parm_types.push_back(parm.type);
                    }

                    return builder.m_type_ctx.get_function(
                        func_def->ret,
                        std::move(parm_types)
                    );
                }
            }
        }
    }
    return nullptr;
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprUnary>(
    IRBuilder& builder,
    const ast::ExprUnary* ast_expr_unary
) noexcept
{
    auto op = to_unary_op(ast_expr_unary->op->kind);
    auto info = UNARY_OP_TABLE[static_cast<uint8_t>(op)];
    auto* type = builder.type_of(ast_expr_unary->expr);
    return info.is_valid(type) ? info.get_result(&builder.m_type_ctx, type) : nullptr;
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprBinary>(
    IRBuilder& builder,
    const ast::ExprBinary* ast_expr_binary
) noexcept
{
    auto op = to_binary_op(ast_expr_binary->op->kind);
    auto info = BINARY_OP_TABLE[static_cast<uint8_t>(op)];
    auto *lhs = builder.type_of(ast_expr_binary->lhs),
         *rhs = builder.type_of(ast_expr_binary->rhs);
    return info.is_valid(lhs, rhs) ? info.get_result(&builder.m_type_ctx, lhs, rhs)
                                   : nullptr;
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprCall>(
    IRBuilder& builder,
    const ast::ExprCall* ast_expr_call
) noexcept
{
    auto* callee = builder.type_of(ast_expr_call->lval);
    if TRY_COERCE (const sema::FuncType, func, callee) {
        return func->result;
    }
    return nullptr;
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprCast>(
    IRBuilder& builder,
    const ast::ExprCast* ast_expr_cast
) noexcept
{
    return builder.type_of(ast_expr_cast->type);
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprTernary>(
    IRBuilder& builder,
    const ast::ExprTernary* ast_expr_ternary
) noexcept
{
    auto *lhs = builder.type_of(ast_expr_ternary->lhs),
         *rhs = builder.type_of(ast_expr_ternary->rhs);
    return lhs == rhs ? lhs : nullptr;
}

template <>
const sema::Type* via::detail::ast_type_of<ast::TypeBuiltin>(
    IRBuilder& builder,
    const ast::TypeBuiltin* ast_type_builtin
) noexcept
{
    using enum TokenKind;
    using enum sema::BuiltinKind;
    sema::BuiltinKind kind;

    switch (ast_type_builtin->tok->kind) {
    case LIT_NIL:
        kind = NIL;
        break;
    case KW_BOOL:
        kind = BOOL;
        break;
    case KW_INT:
        kind = INT;
        break;
    case KW_FLOAT:
        kind = FLOAT;
        break;
    case KW_STRING:
        kind = STRING;
        break;
    default:
        debug::bug("unmapped builtin type token");
    }

    return builder.m_type_ctx.get_builtin(kind);
}

const sema::Type* via::IRBuilder::type_of(const ast::Expr* expr) noexcept
{
#define VISIT_EXPR(TYPE)                                                                 \
    if TRY_COERCE (const TYPE, _INNER, expr)                                             \
        return detail::ast_type_of<TYPE>(*this, _INNER);

    VISIT_EXPR(ast::ExprLiteral)
    VISIT_EXPR(ast::ExprSymbol)
    VISIT_EXPR(ast::ExprStaticAccess)
    VISIT_EXPR(ast::ExprDynAccess)
    VISIT_EXPR(ast::ExprUnary)
    VISIT_EXPR(ast::ExprBinary)
    VISIT_EXPR(ast::ExprCall)
    VISIT_EXPR(ast::ExprSubscript)
    VISIT_EXPR(ast::ExprCast)
    VISIT_EXPR(ast::ExprTernary)
    VISIT_EXPR(ast::ExprArray)
    VISIT_EXPR(ast::ExprTuple)
    VISIT_EXPR(ast::ExprLambda)

    if TRY_COERCE (const ast::ExprGroup, expr_group, expr)
        return type_of(expr_group->expr);

    debug::unimplemented(std::format("ast_type_of({})", VIA_TYPENAME(*expr)));
#undef VISIT_EXPR
}

const sema::Type* via::IRBuilder::type_of(const ast::Type* type) noexcept
{
#define VISIT_TYPE(TYPE)                                                                 \
    if TRY_COERCE (const TYPE, _INNER, type)                                             \
        return detail::ast_type_of<TYPE>(*this, _INNER);

    VISIT_TYPE(ast::TypeBuiltin)
    VISIT_TYPE(ast::TypeArray)
    VISIT_TYPE(ast::TypeDict)
    VISIT_TYPE(ast::TypeFunc)

    debug::todo(std::format("ast_type_of({})", VIA_TYPENAME(*type)));
#undef VISIT_TYPE
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprLiteral>(
    IRBuilder& builder,
    const ast::ExprLiteral* ast_literal_expr
) noexcept
{
    auto const_value = sema::ConstValue::from_token(*ast_literal_expr->tok);

    debug::require(const_value.has_value());

    auto* constant_expr = builder.m_alloc.emplace<ir::ExprConstant>();
    constant_expr->loc = ast_literal_expr->loc;
    constant_expr->value = *const_value;
    constant_expr->type = ast_type_of(builder, ast_literal_expr);
    return constant_expr;
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprSymbol>(
    IRBuilder& builder,
    const ast::ExprSymbol* ast_symbol_expr
) noexcept
{
    auto& frame = builder.m_stack.top();
    auto symbol = ast_symbol_expr->symbol->to_string();
    auto* ast_expr_symbol = builder.m_alloc.emplace<ir::ExprSymbol>();
    ast_expr_symbol->loc = ast_symbol_expr->loc;
    ast_expr_symbol->symbol = builder.m_symbol_table.intern(symbol);
    ast_expr_symbol->type = ast_type_of(builder, ast_symbol_expr);

    if (!frame.get_local(ast_expr_symbol->symbol).has_value()) {
        builder.m_diags.report<Level::ERROR>(
            ast_expr_symbol->loc,
            std::format("Use of undefined symbol '{}'", symbol),
            Footnote(
                FootnoteKind::HINT,
                std::format("did you mistype '{}' or forget to declare it?", symbol)
            )
        );
    }
    return ast_expr_symbol;
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprStaticAccess>(
    IRBuilder& builder,
    const ast::ExprStaticAccess* ast_stc_access_expr
) noexcept
{
    // Check for module thing
    if TRY_COERCE (const ast::ExprSymbol, root_symbol, ast_stc_access_expr->root) {
        auto& manager = builder.m_module->manager();
        auto high = builder.intern_symbol(root_symbol->symbol->to_string());

        if (auto* module = manager.get_module_by_name(high)) {
            auto low = builder.intern_symbol(ast_stc_access_expr->index->to_string());

            if (auto def = module->lookup(low)) {
                auto* maccess = builder.m_alloc.emplace<ir::ExprModuleAccess>();
                maccess->module = module;
                maccess->mod_id = high;
                maccess->key_id = low;
                maccess->def = *def;
                return maccess;
            }
        }
    }

    auto* access_expr = builder.m_alloc.emplace<ir::ExprAccess>();
    access_expr->kind = Ak::STATIC;
    access_expr->root = builder.lower_expr(ast_stc_access_expr->root);
    access_expr->index = builder.intern_symbol(*ast_stc_access_expr->index);
    access_expr->type = ast_type_of(builder, ast_stc_access_expr);
    access_expr->loc = ast_stc_access_expr->loc;
    return access_expr;
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprDynAccess>(
    IRBuilder& builder,
    const ast::ExprDynAccess* ast_dyn_access_expr
) noexcept
{
    auto* access_expr = builder.m_alloc.emplace<ir::ExprAccess>();
    access_expr->kind = Ak::DYNAMIC;
    access_expr->root = builder.lower_expr(ast_dyn_access_expr->root);
    access_expr->index = builder.intern_symbol(*ast_dyn_access_expr->index);
    access_expr->type = ast_type_of(builder, ast_dyn_access_expr);
    access_expr->loc = ast_dyn_access_expr->loc;
    return access_expr;
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprUnary>(
    IRBuilder& builder,
    const ast::ExprUnary* ast_expr_unary
) noexcept
{
    auto* unary_expr = builder.m_alloc.emplace<ir::ExprUnary>();
    unary_expr->op = to_unary_op(ast_expr_unary->op->kind);
    unary_expr->expr = builder.lower_expr(ast_expr_unary->expr);
    unary_expr->loc = ast_expr_unary->loc;

    auto op = to_unary_op(ast_expr_unary->op->kind);
    auto info = UNARY_OP_TABLE[static_cast<uint8_t>(op)];
    auto* type = builder.type_of(ast_expr_unary->expr);

    if (!info.is_valid(type)) {
        builder.m_diags.report<Level::ERROR>(
            ast_expr_unary->loc,
            std::format(
                "Invalid unary operation '{}' ({}) on "
                "incompatible type '{}'",
                ast_expr_unary->op->to_string(),
                to_string(op),
                builder.dump_type(type)
            )
        );
    }

    unary_expr->type = info.get_result(&builder.m_type_ctx, type);
    return unary_expr;
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprBinary>(
    IRBuilder& builder,
    const ast::ExprBinary* ast_expr_binary
) noexcept
{
    auto* binary_expr = builder.m_alloc.emplace<ir::ExprBinary>();
    binary_expr->op = to_binary_op(ast_expr_binary->op->kind);
    binary_expr->lhs = builder.lower_expr(ast_expr_binary->lhs);
    binary_expr->rhs = builder.lower_expr(ast_expr_binary->rhs);
    binary_expr->loc = ast_expr_binary->loc;

    auto op = to_binary_op(ast_expr_binary->op->kind);
    auto info = BINARY_OP_TABLE[static_cast<uint8_t>(op)];
    auto *lhs = builder.type_of(ast_expr_binary->lhs),
         *rhs = builder.type_of(ast_expr_binary->rhs);

    if (!info.is_valid(lhs, rhs)) {
        builder.m_diags.report<Level::ERROR>(
            ast_expr_binary->loc,
            std::format(
                "Invalid binary operation '{}' ({}) on "
                "incompatible types '{}' (LEFT) "
                "'{}' (RIGHT)",
                ast_expr_binary->op->to_string(),
                to_string(op),
                lhs->to_string(),
                rhs->to_string()
            )
        );
    }

    binary_expr->type = info.get_result(&builder.m_type_ctx, lhs, rhs);
    return binary_expr;
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprGroup>(
    IRBuilder& builder,
    const ast::ExprGroup* ast_expr_group
) noexcept
{
    return builder.lower_expr(ast_expr_group->expr);
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprCall>(
    IRBuilder& builder,
    const ast::ExprCall* ast_expr_call
) noexcept
{
    auto* call_expr = builder.m_alloc.emplace<ir::ExprCall>();
    call_expr->callee = builder.lower_expr(ast_expr_call->lval);
    call_expr->loc = ast_expr_call->loc;
    call_expr->args = [&]() {
        std::vector<const ir::Expr*> args;
        for (const auto& ast_arg: ast_expr_call->args) {
            args.push_back(builder.lower_expr(ast_arg));
        }
        return args;
    }();

    auto* callee = builder.type_of(ast_expr_call->lval);

    if TRY_COERCE (const sema::FuncType, func, callee) {
        const size_t arg_count = ast_expr_call->args.size(),
                     parm_count = func->params.size();

        for (size_t arg_id = 0; const sema::Type* parm_type: func->params) {
            if (arg_id >= arg_count) {
                builder.m_diags.report<Level::ERROR>(
                    {ast_expr_call->loc.end - 1, ast_expr_call->loc.end},
                    std::format(
                        "In function call to '{}': missing required argument for "
                        "parameter #{}",
                        builder.dump_expr(ast_expr_call->lval),
                        arg_id
                    )
                );
            } else {
                auto* arg = ast_expr_call->args.at(arg_id);
                auto* arg_type = builder.type_of(arg);
                if (arg_type != parm_type) {
                    builder.m_diags.report<Level::ERROR>(
                        arg->loc,
                        std::format(
                            "In function call to '{}': argument #{} of type '{}' is "
                            "incompatible with parameter that expects type '{}'",
                            builder.dump_expr(ast_expr_call->lval),
                            arg_id,
                            builder.dump_type(arg_type),
                            builder.dump_type(parm_type)
                        ),
                        arg_type->is_castable(parm_type)
                            ? Footnote{
                                FootnoteKind::NOTE,
                                std::format(
                                    "Conversion from '{}' to '{}' possible with explicit cast",
                                    builder.dump_type(arg_type),
                                    builder.dump_type(parm_type)
                                )
                            }
                            : Footnote{}
                    );
                }
            }
            arg_id++;
        }

        if (arg_count > parm_count) {
            auto first = ast_expr_call->args[parm_count];
            auto last = ast_expr_call->args.back();
            builder.m_diags.report<Level::ERROR>(
                {first->loc.begin, last->loc.end},
                std::format(
                    "In function call to '{}': expected {} arguments, got {}",
                    builder.dump_expr(ast_expr_call->lval),
                    parm_count,
                    arg_count
                ),
                {FootnoteKind::SUGGESTION, "Remove argument(s)"}
            );
        }

        call_expr->type = func->result;
    } else {
        builder.m_diags.report<Level::ERROR>(
            ast_expr_call->loc,
            std::format(
                "Attempt to call non-function type '{}'",
                builder.dump_type(callee)
            )
        );
    }
    return call_expr;
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprCast>(
    IRBuilder& builder,
    const ast::ExprCast* ast_expr_cast
) noexcept
{
    auto* cast_type = builder.type_of(ast_expr_cast->type);
    auto* cast_expr = builder.m_alloc.emplace<ir::ExprCast>();
    cast_expr->expr = builder.lower_expr(ast_expr_cast->expr);
    cast_expr->cast = cast_type;
    cast_expr->type = cast_type;

    auto* expr_type = cast_expr->expr->type;
    if (expr_type != nullptr) {
        if (expr_type == cast_type) {
            builder.m_diags.report<Level::WARNING>(
                ast_expr_cast->expr->loc,
                std::format(
                    "Redundant type cast: expression is already of type '{}'",
                    builder.dump_type(cast_type)
                ),
                {FootnoteKind::SUGGESTION, "Remove cast"}
            );
        }

        if (!expr_type->is_castable(cast_type)) {
            builder.m_diags.report<Level::ERROR>(
                ast_expr_cast->expr->loc,
                std::format(
                    "Expression of type '{}' cannot be casted into type '{}'",
                    builder.dump_type(expr_type),
                    builder.dump_type(cast_type)
                )
            );
        }
    }
    return cast_expr;
}

const ir::Expr* via::IRBuilder::lower_expr(const ast::Expr* expr)
{
#define VISIT_EXPR(TYPE)                                                                 \
    if TRY_COERCE (const TYPE, _INNER, expr)                                             \
        return detail::ast_lower_expr<TYPE>(*this, _INNER);

    VISIT_EXPR(ast::ExprLiteral)
    VISIT_EXPR(ast::ExprSymbol)
    VISIT_EXPR(ast::ExprStaticAccess)
    VISIT_EXPR(ast::ExprDynAccess)
    VISIT_EXPR(ast::ExprUnary)
    VISIT_EXPR(ast::ExprBinary)
    VISIT_EXPR(ast::ExprCall)
    VISIT_EXPR(ast::ExprSubscript)
    VISIT_EXPR(ast::ExprCast)
    VISIT_EXPR(ast::ExprTernary)
    VISIT_EXPR(ast::ExprArray)
    VISIT_EXPR(ast::ExprTuple)
    VISIT_EXPR(ast::ExprLambda)

    if TRY_COERCE (const ast::ExprGroup, expr_group, expr)
        return lower_expr(expr_group->expr);

    debug::unimplemented(
        std::format("case IRBuilder::lower_expr({})", VIA_TYPENAME(*expr))
    );
#undef VISIT_EXPR
}

template <>
const ir::Stmt* via::detail::ast_lower_stmt<ast::StmtIf>(
    IRBuilder& builder,
    const ast::StmtIf* ast_stmt_if
) noexcept
{
    auto* merge_block = builder.m_alloc.emplace<ir::StmtBlock>();
    merge_block->id = builder.m_block_id++;

    ir::TrCondBranch* last = nullptr;

    for (size_t i = 0; const auto& branch: ast_stmt_if->brs) {
        auto* then_block = builder.m_alloc.emplace<ir::StmtBlock>();
        then_block->id = builder.m_block_id++;

        auto* then_term = builder.m_alloc.emplace<ir::TrBranch>();
        then_term->target = merge_block;
        then_block->term = then_term;

        auto* current_block = builder.m_current_block;
        builder.m_current_block = then_block;

        for (const auto& stmt: branch.br->stmts) {
            builder.m_current_block->stmts.push_back(builder.lower_stmt(stmt));
        }

        builder.m_current_block = current_block;

        if (branch.cnd != nullptr) {
            auto* cond_block = builder.m_alloc.emplace<ir::StmtBlock>();
            cond_block->id = builder.m_block_id++;

            builder.m_current_block->stmts.push_back(cond_block);
            builder.m_current_block->stmts.push_back(then_block);

            auto* term = builder.m_alloc.emplace<ir::TrCondBranch>();
            term->cnd = builder.lower_expr(branch.cnd);
            term->iftrue = then_block;

            if (i == ast_stmt_if->brs.size() - 1) {
                term->iffalse = merge_block;
            }
            if (last != nullptr) {
                last->iffalse = cond_block;
            }

            last = term;
            cond_block->term = term;
        } else {
            last->iffalse = then_block;
            builder.m_current_block->stmts.push_back(then_block);
        }

        i++;
    }
    return merge_block;
}

template <>
const ir::Stmt* via::detail::ast_lower_stmt<ast::StmtWhile>(
    IRBuilder& builder,
    const ast::StmtWhile* ast_stmt_while
) noexcept
{
    auto* merge_block = builder.m_alloc.emplace<ir::StmtBlock>();
    merge_block->id = builder.m_block_id++;

    auto* cond_block = builder.m_alloc.emplace<ir::StmtBlock>();
    cond_block->id = builder.m_block_id++;

    auto* body_term = builder.m_alloc.emplace<ir::TrBranch>();
    body_term->target = cond_block;

    auto* body_block = builder.m_alloc.emplace<ir::StmtBlock>();
    body_block->id = builder.m_block_id++;
    body_block->term = body_term;

    auto* current_block = builder.m_current_block;
    builder.m_current_block = body_block;

    for (const auto& stmt: ast_stmt_while->br->stmts) {
        builder.m_current_block->stmts.push_back(builder.lower_stmt(stmt));
    }

    builder.m_current_block = current_block;

    auto* cond_term = builder.m_alloc.emplace<ir::TrCondBranch>();
    cond_term->cnd = builder.lower_expr(ast_stmt_while->cnd);
    cond_term->iftrue = body_block;
    cond_term->iffalse = merge_block;
    cond_block->term = cond_term;

    builder.m_current_block->stmts.push_back(cond_block);
    builder.m_current_block->stmts.push_back(body_block);
    return merge_block;
}

template <>
const ir::Stmt* via::detail::ast_lower_stmt<ast::StmtVarDecl>(
    IRBuilder& builder,
    const ast::StmtVarDecl* ast_stmt_var_decl
) noexcept
{
    auto* decl_stmt = builder.m_alloc.emplace<ir::StmtVarDecl>();
    decl_stmt->expr = builder.lower_expr(ast_stmt_var_decl->rval);
    decl_stmt->loc = ast_stmt_var_decl->loc;

    if TRY_COERCE (const ast::ExprSymbol, lval, ast_stmt_var_decl->lval) {
        auto* rval_type = builder.type_of(ast_stmt_var_decl->rval);

        if (ast_stmt_var_decl->type != nullptr) {
            decl_stmt->type = builder.type_of(ast_stmt_var_decl->type);
            if (decl_stmt->type != rval_type) {
                builder.m_diags.report<Level::ERROR>(
                    ast_stmt_var_decl->rval->loc,
                    std::format(
                        "Expression of type '{}' does not match declaration type '{}'",
                        builder.dump_type(rval_type),
                        builder.dump_type(decl_stmt->type)
                    ),
                    rval_type->is_castable(decl_stmt->type)
                        ? Footnote{
                            FootnoteKind::NOTE,
                            std::format(
                                "Conversion from '{}' to '{}' possible with explicit cast",
                                builder.dump_type(rval_type),
                                builder.dump_type(decl_stmt->type)
                            )
                        }
                        : Footnote{}
                );
                goto fallback;
            }
        } else {
fallback:
            decl_stmt->type = rval_type;
        }

        decl_stmt->symbol = builder.intern_symbol(lval->symbol->to_string());
    } else {
        debug::bug("bad lvalue");
    }

    builder.m_stack.top().set_local(decl_stmt->symbol, ast_stmt_var_decl, decl_stmt);
    return decl_stmt;
}

template <>
const ir::Stmt* via::detail::ast_lower_stmt<ast::StmtReturn>(
    IRBuilder& builder,
    const ast::StmtReturn* ast_stmt_return
) noexcept
{
    auto* term = builder.m_alloc.emplace<ir::TrReturn>();
    term->implicit = false;
    term->loc = ast_stmt_return->loc;
    term->val =
        ast_stmt_return->expr ? builder.lower_expr(ast_stmt_return->expr) : nullptr;
    term->type = ast_stmt_return->expr
                     ? builder.type_of(ast_stmt_return->expr)
                     : builder.m_type_ctx.get_builtin(sema::BuiltinKind::NIL);

    auto* block = builder.end_block();
    block->term = term;
    return block;
}

template <>
const ir::Stmt* via::detail::ast_lower_stmt<ast::StmtImport>(
    IRBuilder& builder,
    const ast::StmtImport* ast_stmt_import
) noexcept
{
    if (builder.m_stack.size() > 1) {
        builder.m_diags.report<Level::ERROR>(
            ast_stmt_import->loc,
            "Import statements are only allowed in root scope of a module"
        );
        return nullptr;
    }

    QualName qual_name;
    for (const Token* token: ast_stmt_import->path) {
        qual_name.push_back(token->to_string());
    }

    auto name = qual_name.back();
    if (auto module = builder.m_module->manager().get_module_by_name(name)) {
        builder.m_diags.report<Level::ERROR>(
            ast_stmt_import->loc,
            std::format("Module '{}' imported more than once", name)
        );

        if (auto* import_decl = module->ast_decl()) {
            builder.m_diags.report<Level::INFO>(
                import_decl->loc,
                "Previously imported here"
            );
        }
    }

    auto result = builder.m_module->import(qual_name, ast_stmt_import);
    if (!result.has_value()) {
        builder.m_diags.report<Level::ERROR>(ast_stmt_import->loc, result.error());
    }

    return nullptr;
}

template <>
const ir::Stmt* via::detail::ast_lower_stmt<ast::StmtFunctionDecl>(
    IRBuilder& builder,
    const ast::StmtFunctionDecl* ast_stmt_function_decl
) noexcept
{
    auto* decl_stmt = builder.m_alloc.emplace<ir::StmtFuncDecl>();
    decl_stmt->kind = ir::StmtFuncDecl::Kind::IR;
    decl_stmt->symbol = builder.intern_symbol(ast_stmt_function_decl->name->to_string());
    decl_stmt->ret = ast_stmt_function_decl->ret
                         ? builder.type_of(ast_stmt_function_decl->ret)
                         : nullptr;

    // TODO: Get rid of this
    if (decl_stmt->ret == nullptr) {
        builder.m_diags.report<Level::ERROR>(
            ast_stmt_function_decl->loc,
            "Compiler infered return types are not implemented"
        );
        return nullptr;
    }

    for (const auto& parm: ast_stmt_function_decl->parms) {
        ir::Parm new_parm;
        new_parm.symbol = builder.intern_symbol(parm->symbol->to_string());
        new_parm.type = builder.type_of(parm->type);
        decl_stmt->parms.push_back(new_parm);
    }

    auto* block = builder.m_alloc.emplace<ir::StmtBlock>();
    block->id = builder.m_block_id++;

    builder.m_stack.push({});

    for (const auto& stmt: ast_stmt_function_decl->scope->stmts) {
        if TRY_COERCE (const ast::StmtReturn, ret, stmt) {
            auto* term = builder.m_alloc.emplace<ir::TrReturn>();
            term->implicit = false;
            term->loc = ret->loc;
            term->val = ret->expr ? builder.lower_expr(ret->expr) : nullptr;
            term->type = ret->expr
                             ? builder.type_of(ret->expr)
                             : builder.m_type_ctx.get_builtin(sema::BuiltinKind::NIL);
            block->term = term;
            break;
        }

        block->stmts.push_back(builder.lower_stmt(stmt));
    }

    builder.m_stack.pop();

    if (block->term == nullptr) {
        SourceLoc loc{
            ast_stmt_function_decl->scope->loc.end - 1,
            ast_stmt_function_decl->scope->loc.end
        };

        auto* nil = builder.m_alloc.emplace<ir::ExprConstant>();
        nil->loc = loc;
        nil->type = builder.m_type_ctx.get_builtin(sema::BuiltinKind::NIL);
        nil->value = sema::ConstValue();

        auto* term = builder.m_alloc.emplace<ir::TrReturn>();
        term->implicit = true;
        term->loc = loc;
        term->val = nil;
        term->type = builder.m_type_ctx.get_builtin(sema::BuiltinKind::NIL);
        block->term = term;
    }

    const sema::Type* expected_ret_type = decl_stmt->ret;

    for (const auto& term: sema::get_control_paths(block)) {
        if TRY_COERCE (const ir::TrReturn, ret, term) {
            if (!ret->type) {
                // Already failed, no need to diagnose further
                continue;
            }

            if (!expected_ret_type) {
                expected_ret_type = ret->type;
            } else if (expected_ret_type != ret->type) {
                Footnote implicit_return_node =
                    ret->implicit
                        ? Footnote(
                              FootnoteKind::NOTE,
                              std::format("Implicit return here", ret->type->to_string())
                          )
                        : Footnote();

                if (decl_stmt->ret) {
                    builder.m_diags.report<Level::ERROR>(
                        ret->loc,
                        std::format(
                            "Function return type '{}' does not match type "
                            "'{}' returned by control path",
                            decl_stmt->ret->to_string(),
                            ret->type->to_string()
                        ),
                        implicit_return_node
                    );
                } else {
                    builder.m_diags.report<Level::ERROR>(
                        ret->loc,
                        "All code paths must return the same type "
                        "in function with inferred return type",
                        implicit_return_node
                    );
                }
                break;
            }
        } else {
            builder.m_diags.report<Level::ERROR>(
                term->loc,
                "All control paths must return from function"
            );
            break;
        }
    }

    if (decl_stmt->ret && expected_ret_type && decl_stmt->ret != expected_ret_type) {
        builder.m_diags.report<Level::ERROR>(
            block->loc,
            std::format(
                "Function return type '{}' does not match inferred "
                "return type '{}' from all control paths",
                builder.dump_type(decl_stmt->ret),
                builder.dump_type(expected_ret_type)
            )
        );
    }

    auto& frame = builder.m_stack.top();
    frame.set_local(
        decl_stmt->symbol,
        ast_stmt_function_decl,
        decl_stmt,
        LocalQual::CONST
    );

    decl_stmt->body = block;
    decl_stmt->loc = ast_stmt_function_decl->loc;
    return decl_stmt;
}

template <>
const ir::Stmt* via::detail::ast_lower_stmt<ast::StmtExpr>(
    IRBuilder& builder,
    const ast::StmtExpr* ast_stmt_expr
) noexcept
{
    auto* expr = builder.m_alloc.emplace<ir::StmtExpr>();
    expr->expr = builder.lower_expr(ast_stmt_expr->expr);
    expr->loc = ast_stmt_expr->loc;
    return expr;
}

const ir::Stmt* via::IRBuilder::lower_stmt(const ast::Stmt* stmt)
{
#define VISIT_STMT(TYPE)                                                                 \
    if TRY_COERCE (const TYPE, _INNER, stmt)                                             \
        return detail::ast_lower_stmt<TYPE>(*this, _INNER);

    VISIT_STMT(ast::StmtVarDecl);
    VISIT_STMT(ast::StmtScope);
    VISIT_STMT(ast::StmtIf);
    VISIT_STMT(ast::StmtFor);
    VISIT_STMT(ast::StmtForEach);
    VISIT_STMT(ast::StmtWhile);
    VISIT_STMT(ast::StmtAssign);
    VISIT_STMT(ast::StmtReturn);
    VISIT_STMT(ast::StmtEnum);
    VISIT_STMT(ast::StmtModule);
    VISIT_STMT(ast::StmtImport);
    VISIT_STMT(ast::StmtFunctionDecl);
    VISIT_STMT(ast::StmtStructDecl);
    VISIT_STMT(ast::StmtTypeDecl);
    VISIT_STMT(ast::StmtUsing);
    VISIT_STMT(ast::StmtExpr);

    if TRY_IS (const ast::StmtEmpty, stmt)
        return nullptr;

    debug::unimplemented(
        std::format("case IRBuilder::lower_stmt({})", VIA_TYPENAME(*stmt))
    );
#undef VISIT_STMT
}

via::IRTree via::IRBuilder::build()
{
    m_stack.push({});        // Push root stack frame
    new_block(m_block_id++); // Push block

    IRTree tree;

    for (const auto& astStmt: m_ast) {
        if (const ir::Stmt* loweredStmt = lower_stmt(astStmt)) {
            m_current_block->stmts.push_back(loweredStmt);
        }
        if (m_should_push_block) {
            ir::StmtBlock* block = new_block(m_block_id++);
            tree.push_back(block);
        }
    }

    // Push last block (it likely will not have a terminator)
    tree.push_back(end_block());
    return tree;
}
