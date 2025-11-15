/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "ir.hpp"
#include "lexer/token.hpp"
#include "module/symbol.hpp"
#include "support/ansi.hpp"

#define SYMBOL_ERROR "<symbol error>"
#define EXPR_ERROR "<expression error>"
#define STMT_ERROR "<statement error>"
#define TERM_ERROR "<terminator error>"

#define INDENT(DEPTH) (std::string((DEPTH) * 2, ' '))
#define SYMBOL(ID) (sym_tab->lookup((ID)).value_or(SYMBOL_ERROR))
#define TOSTRING(OBJ, DEPTH, ALT)                                                        \
    ((OBJ) ? (OBJ)->to_string(sym_tab, (DEPTH)) : INDENT((DEPTH)) + (ALT))

using enum via::TokenKind;

via::UnaryOp via::to_unary_op(via::TokenKind kind) noexcept
{
    switch (kind) {
    case OP_MINUS:
        return UnaryOp::NOT;
    case KW_NOT:
        return UnaryOp::NOT;
    case OP_TILDE:
        return UnaryOp::BNOT;
    default:
        break;
    }

    via::debug::unimplemented("unmapped UnaryOp TokenKind");
}

via::BinaryOp via::to_binary_op(via::TokenKind kind) noexcept
{
    switch (kind) {
    case OP_PLUS:
        return BinaryOp::ADD;
    case OP_MINUS:
        return BinaryOp::SUB;
    case OP_STAR:
        return BinaryOp::MUL;
    case OP_SLASH:
        return BinaryOp::DIV;
    case OP_STAR_STAR:
        return BinaryOp::POW;
    case OP_PERCENT:
        return BinaryOp::MOD;
    case KW_AND:
        return BinaryOp::AND;
    case KW_OR:
        return BinaryOp::OR;
    case OP_AMP:
        return BinaryOp::BAND;
    case OP_PIPE:
        return BinaryOp::BOR;
    case OP_CARET:
        return BinaryOp::BXOR;
    case OP_SHL:
        return BinaryOp::BSHL;
    case OP_SHR:
        return BinaryOp::BSHR;
    default:
        break;
    }

    via::debug::unimplemented("unmapped BinaryOp TokenKind");
}

std::string via::ir::TrReturn::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format(
                               "RETURN {} {}",
                               TOSTRING(val, 0, EXPR_ERROR),
                               implicit ? "(implicit)" : ""
                           );
}

std::string via::ir::TrContinue::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + "CONTINUE";
}

std::string via::ir::TrBreak::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + "BREAK";
}

std::string via::ir::TrBranch::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format("BRANCH #{}", target->id);
}

std::string
via::ir::TrCondBranch::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format(
                               "BRANCH {} ? #{} : #{}",
                               TOSTRING(cnd, 0, EXPR_ERROR),
                               iftrue->id,
                               iffalse->id
                           );
}

std::string via::ir::Parameter::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format("{}: {}", SYMBOL(symbol), type.to_string());
}

std::string
via::ir::ExprConstant::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + value.to_string();
}

std::string via::ir::ExprSymbol::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::string(SYMBOL(symbol));
}

std::string via::ir::ExprAccess::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format(
                               "{}{}{}",
                               TOSTRING(root, 0, EXPR_ERROR),
                               kind == Kind::DYNAMIC ? "." : "::",
                               SYMBOL(index)
                           );
}

std::string
via::ir::ExprModuleAccess::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format("MODULE({})::{}", SYMBOL(mod_id), SYMBOL(key_id));
}

std::string via::ir::ExprUnary::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) +
           std::format("({} {})", via::to_string(op), TOSTRING(expr, 0, EXPR_ERROR));
}

std::string via::ir::ExprBinary::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format(
                               "({} {} {})",
                               TOSTRING(lhs, 0, EXPR_ERROR),
                               via::to_string(op),
                               TOSTRING(rhs, 0, EXPR_ERROR)
                           );
}

std::string via::ir::ExprCall::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) +
           std::format(
               "CALL {}{}",
               TOSTRING(callee, 0, EXPR_ERROR),
               via::to_string(
                   args,
                   [&](const auto& expr) { return TOSTRING(expr, 0, EXPR_ERROR); },
                   "(",
                   ")"
               )
           );
}

std::string
via::ir::ExprSubscript::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format(
                               "{}[{}]",
                               TOSTRING(expr, 0, EXPR_ERROR),
                               TOSTRING(idx, 0, EXPR_ERROR)
                           );
}

std::string via::ir::ExprCast::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) +
           std::format("{} AS {}", TOSTRING(expr, 0, EXPR_ERROR), cast.to_string());
}

std::string
via::ir::ExprTernary::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format(
                               "({} ? {} : {})",
                               TOSTRING(cnd, 0, EXPR_ERROR),
                               TOSTRING(iftrue, 0, EXPR_ERROR),
                               TOSTRING(iffalse, 0, EXPR_ERROR)
                           );
}

std::string via::ir::ExprArray::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + via::to_string(exprs, [&](const auto& expr) {
               return TOSTRING(expr, 0, EXPR_ERROR);
           });
}

std::string via::ir::ExprTuple::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + "<tuple>";
}

std::string via::ir::ExprLambda::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + "<lambda>";
}

std::string
via::ir::StmtVarDecl::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + std::format(
                               "LOCAL {}: {} = {}",
                               SYMBOL(symbol),
                               type.to_string(),
                               TOSTRING(expr, 0, EXPR_ERROR)
                           );
}

std::string
via::ir::StmtFuncDecl::to_string(const SymbolTable* sym_tab, size_t depth) const
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

    for (const Stmt* stmt: body->stmts)
        oss << TOSTRING(stmt, depth + 1, STMT_ERROR) << "\n";

    oss << TOSTRING(body->term, depth + 1, TERM_ERROR);
    return oss.str();
}

std::string
via::ir::StmtInstruction::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return INDENT(depth) + instr.to_string(false);
}

std::string via::ir::StmtBlock::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "BLOCK #" << id << ":\n";

    for (const Stmt* stmt: stmts)
        oss << TOSTRING(stmt, depth + 1, STMT_ERROR) << "\n";

    oss << TOSTRING(term, depth + 1, TERM_ERROR);
    return oss.str();
}

std::string via::ir::StmtExpr::to_string(const SymbolTable* sym_tab, size_t depth) const
{
    return TOSTRING(expr, depth, EXPR_ERROR);
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
        oss << (node ? node->to_string(&sym_tab, 1) : INDENT(1) + EXPR_ERROR) << "\n";
    return oss.str();
}
