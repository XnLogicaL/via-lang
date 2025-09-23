/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "builder.h"
#include "ir/ir.h"
#include "sema/control_path.h"
#include "sema/stack.h"
#include "support/math.h"

namespace ir = via::ir;
namespace sema = via::sema;
namespace ast = via::ast;

using Ak = ir::ExprAccess::Kind;
using Btk = sema::BuiltinType::Kind;
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
    using enum sema::BuiltinType::Kind;

    sema::BuiltinType::Kind kind;

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

    // Local variable
    if (auto local =
            frame.get_local(builder.intern_symbol(ast_expr_symbol->symbol->to_string())
            )) {
        auto* irDecl = local->local.get_ir_decl();
        if TRY_COERCE (const ir::StmtVarDecl, varDecl, irDecl) {
            return varDecl->declType;
        }
        else if TRY_COERCE (const ir::StmtFuncDecl, funcDecl, irDecl) {
            std::vector<const sema::Type*> parms;
            for (const auto& parm: funcDecl->parms)
                parms.push_back(parm.type);
            return builder.m_type_ctx.get_function(funcDecl->ret, parms);
        }
    }

    builder.m_diags.report<Level::ERROR>(
        ast_expr_symbol->loc,
        std::format("Use of undefined symbol '{}'", symbol),
        Footnote(
            Footnote::Kind::HINT,
            std::format("did you mistype '{}' or forget to declare it?", symbol)
        )
    );
    return nullptr;
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprStaticAccess>(
    IRBuilder& builder,
    const ast::ExprStaticAccess* ast_expr_st_access
) noexcept
{
    return builder.m_type_ctx.get_builtin(Btk::NIL);
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprUnary>(
    IRBuilder& builder,
    const ast::ExprUnary* ast_expr_unary
) noexcept
{
    auto* inner = builder.type_of(ast_expr_unary->expr);
    UnaryOp op = to_unary_op(ast_expr_unary->op->kind);
    UnaryOpInfo info = UNARY_OP_TABLE[static_cast<u8>(op)];

    if (!info.is_valid(inner)) {
        builder.m_diags.report<Level::ERROR>(
            ast_expr_unary->loc,
            std::format(
                "Invalid unary operation '{}' ({}) on "
                "incompatible type '{}'",
                ast_expr_unary->op->to_string(),
                magic_enum::enum_name(op),
                inner->to_string()
            )
        );
        return nullptr;
    }

    return info.get_result(&builder.m_type_ctx, inner);
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprBinary>(
    IRBuilder& builder,
    const ast::ExprBinary* ast_expr_binary
) noexcept
{
    auto* lhs = builder.type_of(ast_expr_binary->lhs);
    auto* rhs = builder.type_of(ast_expr_binary->rhs);

    BinaryOp op = to_binary_op(ast_expr_binary->op->kind);
    BinaryOpInfo info = BINARY_OP_TABLE[static_cast<u8>(op)];

    if (!info.is_valid(lhs, rhs)) {
        builder.m_diags.report<Level::ERROR>(
            ast_expr_binary->loc,
            std::format(
                "Invalid binary operation '{}' ({}) on "
                "incompatible types '{}' (LEFT) "
                "'{}' (RIGHT)",
                ast_expr_binary->op->to_string(),
                magic_enum::enum_name(op),
                lhs->to_string(),
                rhs->to_string()
            )
        );
        return nullptr;
    }

    return info.get_result(&builder.m_type_ctx, lhs, rhs);
}

template <>
const sema::Type* via::detail::ast_type_of<ast::ExprTernary>(
    IRBuilder& builder,
    const ast::ExprTernary* ast_expr_ternary
) noexcept
{
    const sema::Type* lhs = builder.type_of(ast_expr_ternary->lhs);
    const sema::Type* rhs = builder.type_of(ast_expr_ternary->rhs);
    if (lhs == rhs) {
        return lhs;
    }

    builder.m_diags.report<Level::ERROR>(
        ast_expr_ternary->loc,
        std::format(
            "Results of ternary expression '{}' and '{}' do not match",
            lhs->to_string(),
            rhs->to_string()
        )
    );
    return nullptr;
}

template <>
const sema::Type* via::detail::ast_type_of<ast::TypeBuiltin>(
    IRBuilder& builder,
    const ast::TypeBuiltin* ast_type_builtin
) noexcept
{
    using enum TokenKind;
    using enum sema::BuiltinType::Kind;
    sema::BuiltinType::Kind kind;

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
    VISIT_EXPR(ast::ExprGroup)
    VISIT_EXPR(ast::ExprCall)
    VISIT_EXPR(ast::ExprSubscript)
    VISIT_EXPR(ast::ExprCast)
    VISIT_EXPR(ast::ExprTernary)
    VISIT_EXPR(ast::ExprArray)
    VISIT_EXPR(ast::ExprTuple)
    VISIT_EXPR(ast::ExprLambda)

    debug::unimplemented(std::format("ast_type_of({})", TYPENAME(*expr)));
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

    debug::todo(std::format("ast_type_of({})", TYPENAME(*type)));
#undef VISIT_TYPE
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprLiteral>(
    IRBuilder& builder,
    const ast::ExprLiteral* ast_literal_expr
) noexcept
{
    auto* constant_expr = builder.m_alloc.emplace<ir::ExprConstant>();
    constant_expr->loc = ast_literal_expr->loc;
    constant_expr->value = *sema::ConstValue::from_token(*ast_literal_expr->tok);
    constant_expr->type = builder.type_of(ast_literal_expr);
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
    ast_expr_symbol->type = builder.type_of(ast_symbol_expr);
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
        ModuleManager* manager = builder.m_module->get_manager();

        if (auto* module =
                manager->get_module_by_name(root_symbol->symbol->to_string())) {
            SymbolId low = builder.intern_symbol(ast_stc_access_expr->index->to_string());
            if (auto def = module->lookup(low)) {
                auto* maccess = builder.m_alloc.emplace<ir::ExprModuleAccess>();
                maccess->module = module;
                maccess->index = low;
                maccess->def = *def;
                return maccess;
            }
        }
    }

    auto* access_expr = builder.m_alloc.emplace<ir::ExprAccess>();
    access_expr->kind = Ak::STATIC;
    access_expr->root = builder.lower_expr(ast_stc_access_expr->root);
    access_expr->index = builder.intern_symbol(*ast_stc_access_expr->index);
    access_expr->type = builder.type_of(ast_stc_access_expr);
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
    access_expr->type = builder.type_of(ast_dyn_access_expr);
    access_expr->loc = ast_dyn_access_expr->loc;
    return access_expr;
}

template <>
const ir::Expr* via::detail::ast_lower_expr<ast::ExprUnary>(
    IRBuilder& builder,
    const ast::ExprUnary* ast_unary_expr
) noexcept
{
    auto* unary_expr = builder.m_alloc.emplace<ir::ExprUnary>();
    unary_expr->op = to_unary_op(ast_unary_expr->op->kind);
    unary_expr->type = builder.type_of(ast_unary_expr);
    unary_expr->expr = builder.lower_expr(ast_unary_expr->expr);
    unary_expr->loc = ast_unary_expr->loc;
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
    binary_expr->type = builder.type_of(ast_expr_binary);
    binary_expr->loc = ast_expr_binary->loc;
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
    call_expr->type = nullptr; /* TODO: Type of this expression should be the return
                             type of the callee function. */
    call_expr->args = [&]() {
        std::vector<const ir::Expr*> args;
        for (const auto& astArg: ast_expr_call->args) {
            args.push_back(builder.lower_expr(astArg));
        }
        return args;
    }();
    return call_expr;
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
    VISIT_EXPR(ast::ExprGroup)
    VISIT_EXPR(ast::ExprCall)
    VISIT_EXPR(ast::ExprSubscript)
    VISIT_EXPR(ast::ExprCast)
    VISIT_EXPR(ast::ExprTernary)
    VISIT_EXPR(ast::ExprArray)
    VISIT_EXPR(ast::ExprTuple)
    VISIT_EXPR(ast::ExprLambda)

    debug::unimplemented(std::format("case IRBuilder::lower_expr({})", TYPENAME(*expr)));
#undef VISIT_EXPR
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
        auto* rvalType = builder.type_of(ast_stmt_var_decl->rval);

        if (ast_stmt_var_decl->type != nullptr) {
            decl_stmt->declType = builder.type_of(ast_stmt_var_decl->type);
            if (decl_stmt->declType != rvalType) {
                builder.m_diags.report<Level::ERROR>(
                    ast_stmt_var_decl->rval->loc,
                    std::format(
                        "Expression type '{}' does not match declaration type '{}'",
                        rvalType->to_string(),
                        decl_stmt->declType->to_string()
                    )
                );
            }
        }
        else {
            decl_stmt->declType = rvalType;
        }

        decl_stmt->symbol = builder.intern_symbol(lval->symbol->to_string());
    }
    else {
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
                     : builder.m_type_ctx.get_builtin(sema::BuiltinType::Kind::NIL);

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
    if (auto module = builder.m_module->get_manager()->get_module_by_name(name)) {
        builder.m_diags.report<Level::ERROR>(
            ast_stmt_import->loc,
            std::format("Module '{}' imported more than once", name)
        );

        if (auto* import_decl = module->get_ast_decl()) {
            builder.m_diags.report<Level::INFO>(
                import_decl->loc,
                "Previously imported here"
            );
        }
    }

    auto result = builder.m_module->import(qual_name, ast_stmt_import);
    if (result.has_error()) {
        builder.m_diags.report<Level::ERROR>(
            ast_stmt_import->loc,
            result.get_error().to_string()
        );
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
    block->id = iota<size_t>();

    builder.m_stack.push({});

    for (const auto& stmt: ast_stmt_function_decl->scope->stmts) {
        if TRY_COERCE (const ast::StmtReturn, ret, stmt) {
            auto* term = builder.m_alloc.emplace<ir::TrReturn>();
            term->implicit = false;
            term->loc = ret->loc;
            term->val = ret->expr ? builder.lower_expr(ret->expr) : nullptr;
            term->type =
                ret->expr ? builder.type_of(ret->expr)
                          : builder.m_type_ctx.get_builtin(sema::BuiltinType::Kind::NIL);
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
        nil->type = builder.m_type_ctx.get_builtin(sema::BuiltinType::Kind::NIL);
        nil->value = sema::ConstValue();

        auto* term = builder.m_alloc.emplace<ir::TrReturn>();
        term->implicit = true;
        term->loc = loc;
        term->val = nil;
        term->type = builder.m_type_ctx.get_builtin(sema::BuiltinType::Kind::NIL);
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
            }
            else if (expected_ret_type != ret->type) {
                Footnote implicitReturnNote =
                    ret->implicit
                        ? Footnote(
                              Footnote::Kind::NOTE,
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
                        implicitReturnNote
                    );
                }
                else {
                    builder.m_diags.report<Level::ERROR>(
                        ret->loc,
                        "All code paths must return the same type "
                        "in function with inferred return type",
                        implicitReturnNote
                    );
                }
                break;
            }
        }
        else {
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
                decl_stmt->ret->to_string(),
                expected_ret_type->to_string()
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

    if TRY_COERCE (const ast::StmtEmpty, _, stmt)
        return nullptr;

    debug::unimplemented(std::format("case IRBuilder::lower_stmt({})", TYPENAME(*stmt)));
#undef VISIT_STMT
}

via::IRTree via::IRBuilder::build()
{
    m_stack.push({});          // Push root stack frame
    new_block(iota<size_t>()); // Push block

    IRTree tree;

    for (const auto& astStmt: m_ast) {
        if (const ir::Stmt* loweredStmt = lower_stmt(astStmt)) {
            m_current_block->stmts.push_back(loweredStmt);
        }

        if (m_should_push_block) {
            ir::StmtBlock* block = new_block(iota<size_t>());
            tree.push_back(block);
        }
    }

    // Push last block (it likely will not have a terminator)
    tree.push_back(end_block());
    return tree;
}
