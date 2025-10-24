/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "ir.hpp"
#include "module/symbol.hpp"
#include "support/ansi.hpp"

namespace ir = via::ir;
using Tk = via::TokenKind;

#define INDENT(DEPTH) (std::string(DEPTH * 2, ' '))
#define SYMBOL(ID) (sym_tab->lookup(ID).value_or("<symbol error>"))

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

std::string ir::TrReturn::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format(
                               "RETURN {} {}",
                               val ? val->to_string(sym_tab) : "<expression error>",
                               implicit ? "(implicit)" : ""
                           );
}

std::string ir::TrContinue::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + "CONTINUE";
}

std::string ir::TrBreak::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + "BREAK";
}

std::string ir::TrBranch::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format("BRANCH #{}", target->id);
}

std::string ir::TrCondBranch::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format(
                               "BRANCH {} ? #{} : #{}",
                               cnd ? cnd->to_string(sym_tab) : "<expression error>",
                               iftrue->id,
                               iffalse->id
                           );
}

std::string ir::Parameter::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return std::format("{}: {}", SYMBOL(symbol), type.to_string());
}

std::string ir::ExprConstant::to_string(const SymbolTable* sym_tab, size_t) const
{
    return value.to_string();
}

std::string ir::ExprSymbol::to_string(const SymbolTable* sym_tab, size_t) const
{
    return std::string(SYMBOL(symbol));
}

std::string ir::ExprAccess::to_string(const SymbolTable* sym_tab, size_t) const
{
    return std::format(
        "{}{}{}",
        root ? root->to_string(sym_tab) : "<expression error>",
        kind == Kind::DYNAMIC ? "." : "::",
        SYMBOL(index)
    );
}

std::string ir::ExprModuleAccess::to_string(const SymbolTable* sym_tab, size_t) const
{
    return std::format("MODULE({})::{}", SYMBOL(mod_id), SYMBOL(key_id));
}

std::string ir::ExprUnary::to_string(const SymbolTable* sym_tab, size_t) const
{
    return std::format(
        "({} {})",
        via::to_string(op),
        expr ? expr->to_string(sym_tab) : "<expression error>"
    );
}

std::string ir::ExprBinary::to_string(const SymbolTable* sym_tab, size_t) const
{
    return std::format(
        "({} {} {})",
        lhs ? lhs->to_string(sym_tab) : "<expression error>",
        via::to_string(op),
        rhs ? rhs->to_string(sym_tab) : "<expression error>"
    );
}

std::string ir::ExprCall::to_string(const SymbolTable* sym_tab, size_t) const
{
    return std::format(
        "CALL {}{}",
        callee ? callee->to_string(sym_tab) : "<expression error>",
        via::to_string(
            args,
            [&](const auto& expr) {
                return expr ? expr->to_string(sym_tab) : "<expression error>";
            },
            "(",
            ")"
        )
    );
}

std::string ir::ExprSubscript::to_string(const SymbolTable* sym_tab, size_t) const
{
    return std::format(
        "{}[{}]",
        expr ? expr->to_string(sym_tab) : "<expression error>",
        idx ? idx->to_string(sym_tab) : "<expression error>"
    );
}

std::string ir::ExprCast::to_string(const SymbolTable* sym_tab, size_t) const
{
    return std::format(
        "{} AS {}",
        expr ? expr->to_string(sym_tab) : "<expression error>",
        cast.to_string()
    );
}

std::string ir::ExprTernary::to_string(const SymbolTable* sym_tab, size_t) const
{
    return std::format(
        "({} ? {} : {})",
        cnd ? cnd->to_string(sym_tab) : "<expression error>",
        iftrue ? iftrue->to_string(sym_tab) : "<expression error>",
        iffalse ? iffalse->to_string(sym_tab) : "<expression error>"
    );
}

std::string ir::ExprArray::to_string(const SymbolTable* sym_tab, size_t) const
{
    return via::to_string(exprs, [&](const auto& expr) {
        return expr ? expr->to_string(sym_tab) : "<expression error>";
    });
}

std::string ir::ExprTuple::to_string(const SymbolTable* sym_tab, size_t) const
{
    return "<tuple>";
}

std::string ir::ExprLambda::to_string(const SymbolTable* sym_tab, size_t) const
{
    return "<lambda>";
}

std::string ir::StmtVarDecl::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format(
                               "LOCAL {}: {} = {}",
                               SYMBOL(symbol),
                               type.to_string(),
                               expr ? expr->to_string(sym_tab) : "<expression error>"
                           );
}

std::string ir::StmtFuncDecl::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth)
        << std::format(
               "FUNCTION {} {} -> {}:\n",
               SYMBOL(symbol),
               via::to_string(
                   parms,
                   [&](const auto& parm) { return parm.to_string(sym_tab); },
                   "(",
                   ")"
               ),
               ret.to_string()
           );
    oss << INDENT(depth) << "{\n";

    for (const Stmt* stmt: body->stmts)
        oss << (stmt ? stmt->to_string(sym_tab, depth + 1) : "<statement error>") << "\n";

    oss << INDENT(depth + 1)
        << (body->term ? body->term->to_string(sym_tab, depth + 1) : "<terminator error>")
        << "\n";
    oss << INDENT(depth) << "}";
    return oss.str();
}

std::string ir::StmtInstruction::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + instr.to_string(false);
}

std::string ir::StmtBlock::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "BLOCK #" << id << ":\n";

    for (const Stmt* stmt: stmts)
        oss << (stmt ? stmt->to_string(sym_tab, depth + 1) : "<statement error>") << "\n";

    oss << INDENT(depth + 1) << (term ? term->to_string(sym_tab) : "<no terminator>");
    return oss.str();
}

std::string ir::StmtExpr::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + (expr ? expr->to_string(sym_tab) : "<expression error>");
}

std::string via::to_string(const SymbolTable& sym_tab, const IRTree& ir_tree)
{
    std::ostringstream oss;
    oss << ansi::format(
        "[disassembly of program IR]:\n",
        ansi::Foreground::YELLOW,
        ansi::Background::NONE,
        ansi::Style::UNDERLINE
    );

    for (const auto& node: ir_tree)
        oss << (node ? node->to_string(&sym_tab, 1) : "<expression error>") << "\n";
    return oss.str();
}
