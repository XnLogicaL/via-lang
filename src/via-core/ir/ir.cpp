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

inline via::usize ZERO = 0;

#define INDENT std::string(depth * 2, ' ')
#define SYMBOL(ID) (symtab->lookup(ID).value_or("<unknown-symbol>"))
#define DUMP_IF(PTR, ...)             \
    (PTR ? PTR->get_dump(__VA_ARGS__) \
         : [](const SymbolTable* symtab = nullptr, usize& depth = ZERO) { return INDENT + "<null>"; }(__VA_ARGS__))

std::string ir::TrReturn::get_dump(const SymbolTable* symtab, usize& depth) const
{
    return INDENT + std::format("return {} {}", DUMP_IF(val, symtab, ZERO), implicit ? "(implicit)" : "");
}

std::string ir::TrContinue::get_dump(const SymbolTable* symtab, usize& depth) const
{
    return INDENT + "continue";
}

std::string ir::TrBreak::get_dump(const SymbolTable* symtab, usize& depth) const
{
    return INDENT + "break";
}

std::string ir::TrBranch::get_dump(const SymbolTable* symtab, usize& depth) const
{
    return INDENT + std::format("br {}", target->id);
}

std::string ir::TrCondBranch::get_dump(const SymbolTable* symtab, usize& depth) const
{
    return INDENT + std::format("cndbr {} ? .LB{} : .LB{}", DUMP_IF(cnd, symtab, ZERO), iftrue->id, iffalse->id);
}

std::string ir::Parm::get_dump(const SymbolTable* symtab, usize& depth) const
{
    return std::format("{}: {}", SYMBOL(symbol), DUMP_IF(type));
}

std::string ir::ExprConstant::get_dump(const SymbolTable* symtab, usize&) const
{
    return value.get_dump();
}

std::string ir::ExprSymbol::get_dump(const SymbolTable* symtab, usize&) const
{
    return std::string(SYMBOL(symbol));
}

std::string ir::ExprAccess::get_dump(const SymbolTable* symtab, usize&) const
{
    return std::format("{}{}{}", DUMP_IF(root, symtab, ZERO), kind == Kind::DYNAMIC ? "." : "::", SYMBOL(index));
}

std::string ir::ExprModuleAccess::get_dump(const SymbolTable* symtab, usize&) const
{
    return std::format("module<{}>::{} def@{}", module->name(), SYMBOL(index), reinterpret_cast<const void*>(def));
}

std::string ir::ExprUnary::get_dump(const SymbolTable* symtab, usize&) const
{
    return std::format("({} {})", magic_enum::enum_name(op), DUMP_IF(expr, symtab, ZERO));
}

std::string ir::ExprBinary::get_dump(const SymbolTable* symtab, usize&) const
{
    return std::format("({} {} {})", DUMP_IF(lhs, symtab, ZERO), magic_enum::enum_name(op), DUMP_IF(rhs, symtab, ZERO));
}

std::string ir::ExprCall::get_dump(const SymbolTable* symtab, usize&) const
{
    return std::format(
        "call( {}, args: {} )",
        DUMP_IF(callee, symtab, ZERO),
        debug::get_dump<const Expr*>(args, [&](const auto& expr) { return DUMP_IF(expr, symtab, ZERO); })
    );
}

std::string ir::ExprSubscript::get_dump(const SymbolTable* symtab, usize&) const
{
    return std::format("subscr( {}, {} )", DUMP_IF(expr, symtab, ZERO), DUMP_IF(idx, symtab, ZERO));
}

std::string ir::ExprCast::get_dump(const SymbolTable* symtab, usize&) const
{
    return std::format("({} as {})", DUMP_IF(expr, symtab, ZERO), "");
}

std::string ir::ExprTernary::get_dump(const SymbolTable* symtab, usize&) const
{
    return std::format(
        "({} ? {} : {})",
        DUMP_IF(cnd, symtab, ZERO),
        DUMP_IF(iftrue, symtab, ZERO),
        DUMP_IF(iffalse, symtab, ZERO)
    );
}

std::string ir::ExprArray::get_dump(const SymbolTable* symtab, usize&) const
{
    return debug::get_dump<const Expr*, '[', ']'>(exprs, [&](const auto& expr) { return DUMP_IF(expr, symtab, ZERO); });
}

std::string ir::ExprTuple::get_dump(const SymbolTable* symtab, usize&) const
{
    return "<tuple>";
}

std::string ir::ExprLambda::get_dump(const SymbolTable* symtab, usize&) const
{
    return "<lambda>";
}

std::string ir::StmtVarDecl::get_dump(const SymbolTable* symtab, usize& depth) const
{
    return INDENT + std::format("local {}: {} = {}", SYMBOL(symbol), DUMP_IF(declType), DUMP_IF(expr, symtab, ZERO));
}

std::string ir::StmtFuncDecl::get_dump(const SymbolTable* symtab, usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT
        << std::format(
               "function {} {} -> {}:\n",
               SYMBOL(symbol),
               debug::get_dump<Parm, '(', ')'>(parms, [&](const auto& parm) { return parm.get_dump(symtab, ZERO); }),
               ret->get_dump()
           );
    oss << INDENT << "{\n";
    depth++;

    for (const Stmt* stmt: body->stmts) {
        oss << DUMP_IF(stmt, symtab, depth) << "\n";
    }

    oss << INDENT << DUMP_IF(body->term, symtab, ZERO) << "\n";
    depth--;
    oss << INDENT << "}";
    return oss.str();
}

std::string ir::StmtBlock::get_dump(const SymbolTable* symtab, usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT << "block .LB" << id << ":\n";
    oss << INDENT << "{\n";
    depth++;

    for (const Stmt* stmt: stmts) {
        oss << DUMP_IF(stmt, symtab, depth) << "\n";
    }

    oss << INDENT << (term ? DUMP_IF(term, symtab, ZERO) : "<no-terminator>") << "\n";
    depth--;
    oss << INDENT << "}";
    return oss.str();
}

std::string ir::StmtExpr::get_dump(const SymbolTable* symtab, usize& depth) const
{
    return INDENT + DUMP_IF(expr, symtab, ZERO);
}

[[nodiscard]] std::string via::debug::get_dump(const SymbolTable& symtab, const IRTree& ir)
{
    std::ostringstream oss;
    usize depth = 0;

    for (const auto& node: ir) {
        oss << DUMP_IF(node, &symtab, depth) << "\n";
    }

    return oss.str();
}
