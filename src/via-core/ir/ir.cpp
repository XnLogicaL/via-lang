/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "ir.h"
#include "module/module.h"

namespace ir = via::ir;
using Tk = via::TokenKind;

inline size_t ZERO = 0;

#define INDENT std::string(depth * 2, ' ')
#define SYMBOL(ID) (sym_tab->lookup(ID).value_or("<unknown-symbol>"))
#define DUMP_IF(PTR, ...)                                                                \
    (PTR ? PTR->to_string(__VA_ARGS__)                                                   \
         : [](const SymbolTable* sym_tab = nullptr, size_t& depth = ZERO) {              \
               return INDENT + "<null>";                                                 \
           }(__VA_ARGS__))

std::string ir::TrReturn::to_string(const SymbolTable* sym_tab, size_t& depth) const
{
    return INDENT + std::format(
                        "return {} {}",
                        DUMP_IF(val, sym_tab, ZERO),
                        implicit ? "(implicit)" : ""
                    );
}

via::UnaryOp via::to_unary_op(Tk kind) noexcept
{
    switch (kind) {
    case Tk::OP_MINUS:
        return UnaryOp::NOT;
    case Tk::KW_NOT:
        return UnaryOp::NOT;
    case Tk::OP_TILDE:
        return UnaryOp::BNOT;
    default:
        break;
    }

    via::debug::unimplemented("unmapped UnaryOp TokenKind");
}

via::BinaryOp via::to_binary_op(Tk kind) noexcept
{
    switch (kind) {
    case Tk::OP_PLUS:
        return BinaryOp::ADD;
    case Tk::OP_MINUS:
        return BinaryOp::SUB;
    case Tk::OP_STAR:
        return BinaryOp::MUL;
    case Tk::OP_SLASH:
        return BinaryOp::DIV;
    case Tk::OP_STAR_STAR:
        return BinaryOp::POW;
    case Tk::OP_PERCENT:
        return BinaryOp::MOD;
    case Tk::KW_AND:
        return BinaryOp::AND;
    case Tk::KW_OR:
        return BinaryOp::OR;
    case Tk::OP_AMP:
        return BinaryOp::BAND;
    case Tk::OP_PIPE:
        return BinaryOp::BOR;
    case Tk::OP_CARET:
        return BinaryOp::BXOR;
    case Tk::OP_SHL:
        return BinaryOp::BSHL;
    case Tk::OP_SHR:
        return BinaryOp::BSHR;
    default:
        break;
    }

    via::debug::unimplemented("unmapped BinaryOp TokenKind");
}

std::string ir::TrContinue::to_string(const SymbolTable* sym_tab, size_t& depth) const
{
    return INDENT + "continue";
}

std::string ir::TrBreak::to_string(const SymbolTable* sym_tab, size_t& depth) const
{
    return INDENT + "break";
}

std::string ir::TrBranch::to_string(const SymbolTable* sym_tab, size_t& depth) const
{
    return INDENT + std::format("br {}", target->id);
}

std::string ir::TrCondBranch::to_string(const SymbolTable* sym_tab, size_t& depth) const
{
    return INDENT + std::format(
                        "cndbr {} ? .LB{} : .LB{}",
                        DUMP_IF(cnd, sym_tab, ZERO),
                        iftrue->id,
                        iffalse->id
                    );
}

std::string ir::Parm::to_string(const SymbolTable* sym_tab, size_t& depth) const
{
    return std::format("{}: {}", SYMBOL(symbol), DUMP_IF(type));
}

std::string ir::ExprConstant::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return value.to_string();
}

std::string ir::ExprSymbol::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return std::string(SYMBOL(symbol));
}

std::string ir::ExprAccess::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return std::format(
        "{}{}{}",
        DUMP_IF(root, sym_tab, ZERO),
        kind == Kind::DYNAMIC ? "." : "::",
        SYMBOL(index)
    );
}

std::string ir::ExprModuleAccess::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return std::format(
        "module<{}>::{} def@{}",
        SYMBOL(mod_id),
        SYMBOL(key_id),
        reinterpret_cast<const void*>(def)
    );
}

std::string ir::ExprUnary::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return std::format("({} {})", via::to_string(op), DUMP_IF(expr, sym_tab, ZERO));
}

std::string ir::ExprBinary::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return std::format(
        "({} {} {})",
        DUMP_IF(lhs, sym_tab, ZERO),
        via::to_string(op),
        DUMP_IF(rhs, sym_tab, ZERO)
    );
}

std::string ir::ExprCall::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return std::format(
        "call( {}, args: {} )",
        DUMP_IF(callee, sym_tab, ZERO),
        debug::to_string<const Expr*>(
            args,
            [&](const auto& expr) { return DUMP_IF(expr, sym_tab, ZERO); }
        )
    );
}

std::string ir::ExprSubscript::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return std::format(
        "subscr( {}, {} )",
        DUMP_IF(expr, sym_tab, ZERO),
        DUMP_IF(idx, sym_tab, ZERO)
    );
}

std::string ir::ExprCast::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return std::format("({} as {})", DUMP_IF(expr, sym_tab, ZERO), DUMP_IF(cast));
}

std::string ir::ExprTernary::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return std::format(
        "({} ? {} : {})",
        DUMP_IF(cnd, sym_tab, ZERO),
        DUMP_IF(iftrue, sym_tab, ZERO),
        DUMP_IF(iffalse, sym_tab, ZERO)
    );
}

std::string ir::ExprArray::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return debug::to_string<const Expr*, '[', ']'>(exprs, [&](const auto& expr) {
        return DUMP_IF(expr, sym_tab, ZERO);
    });
}

std::string ir::ExprTuple::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return "<tuple>";
}

std::string ir::ExprLambda::to_string(const SymbolTable* sym_tab, size_t&) const
{
    return "<lambda>";
}

std::string ir::StmtVarDecl::to_string(const SymbolTable* sym_tab, size_t& depth) const
{
    return INDENT + std::format(
                        "local {}: {} = {}",
                        SYMBOL(symbol),
                        DUMP_IF(type),
                        DUMP_IF(expr, sym_tab, ZERO)
                    );
}

std::string ir::StmtFuncDecl::to_string(const SymbolTable* sym_tab, size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT
        << std::format(
               "function {} {} -> {}:\n",
               SYMBOL(symbol),
               debug::to_string<Parm, '(', ')'>(
                   parms,
                   [&](const auto& parm) { return parm.to_string(sym_tab, ZERO); }
               ),
               ret->to_string()
           );
    oss << INDENT << "{\n";
    depth++;

    for (const Stmt* stmt: body->stmts) {
        oss << DUMP_IF(stmt, sym_tab, depth) << "\n";
    }

    oss << INDENT << DUMP_IF(body->term, sym_tab, ZERO) << "\n";
    depth--;
    oss << INDENT << "}";
    return oss.str();
}

std::string ir::StmtBlock::to_string(const SymbolTable* sym_tab, size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT << "block .LB" << id << ":\n";
    oss << INDENT << "{\n";
    depth++;

    for (const Stmt* stmt: stmts) {
        oss << DUMP_IF(stmt, sym_tab, depth) << "\n";
    }

    oss << INDENT << (term ? DUMP_IF(term, sym_tab, ZERO) : "<no-terminator>") << "\n";
    depth--;
    oss << INDENT << "}";
    return oss.str();
}

std::string ir::StmtExpr::to_string(const SymbolTable* sym_tab, size_t& depth) const
{
    return INDENT + DUMP_IF(expr, sym_tab, ZERO);
}

[[nodiscard]] std::string
via::debug::to_string(const SymbolTable& sym_tab, const IRTree& ir_tree)
{
    std::ostringstream oss;
    size_t depth = 0;

    for (const auto& node: ir_tree) {
        oss << DUMP_IF(node, &sym_tab, depth) << "\n";
    }

    return oss.str();
}
