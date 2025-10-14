/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "ast.hpp"
#include "debug.hpp"

namespace ast = via::ast;

#define INDENT(depth) std::string(depth * 2, ' ')

inline size_t ZERO = 0;

std::string ast::Path::to_string() const
{
    return std::format("Path({})", debug::to_string(path, [](const auto& node) {
                           return node->to_string();
                       }));
}

std::string ast::Parameter::to_string() const
{
    return std::format(
        "Parameter(symbol={}, type={})",
        symbol->to_string(),
        type->to_string(ZERO)
    );
}

std::string ast::ExprLiteral::to_string(size_t) const
{
    return std::format("ExprLiteral({})", tok->to_string());
}

std::string ast::ExprSymbol::to_string(size_t) const
{
    return std::format("ExprSymbol({})", symbol->to_string());
}

std::string ast::ExprDynAccess::to_string(size_t depth) const
{
    return std::format(
        "ExprDynAccess(root={}, index={})",
        root->to_string(depth + 1),
        index->to_string()
    );
}

std::string ast::ExprStaticAccess::to_string(size_t depth) const
{
    return std::format(
        "ExprStaticAccess(root={}, index={})",
        root->to_string(depth + 1),
        index->to_string()
    );
}

std::string ast::ExprUnary::to_string(size_t depth) const
{
    return std::format(
        "ExprUnary(op={}, expr={})",
        op->to_string(),
        expr->to_string(depth + 1)
    );
}

std::string ast::ExprBinary::to_string(size_t depth) const
{
    return std::format(
        "ExprBinary(op={}, lhs={}, rhs={})",
        op->to_string(),
        lhs->to_string(depth + 1),
        rhs->to_string(depth + 1)
    );
}

std::string ast::ExprGroup::to_string(size_t depth) const
{
    return std::format("ExprGroup({})", expr->to_string(depth + 1));
}

std::string ast::ExprCall::to_string(size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "ExprCall:\n";
    oss << INDENT(depth + 1) << "Callee: " << lval->to_string(depth + 2) << "\n";
    oss << INDENT(depth + 1) << "Args:\n";
    for (auto& arg: args) {
        oss << INDENT(depth + 2) << arg->to_string(depth + 3) << "\n";
    }
    return oss.str();
}

std::string ast::ExprSubscript::to_string(size_t depth) const
{
    return std::format(
        "ExprSubscript(lval={}, idx={})",
        lval->to_string(depth + 1),
        idx->to_string(depth + 1)
    );
}

std::string ast::ExprCast::to_string(size_t depth) const
{
    return std::format(
        "ExprCast(expr={}, to={})",
        expr->to_string(depth + 1),
        type->to_string(depth + 1)
    );
}

std::string ast::ExprTernary::to_string(size_t depth) const
{
    return std::format(
        "ExprTernary(cond={}, lhs={}, rhs={})",
        cnd->to_string(depth + 1),
        lhs->to_string(depth + 1),
        rhs->to_string(depth + 1)
    );
}

std::string ast::ExprArray::to_string(size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "ExprArray:\n";
    for (auto& elem: init) {
        oss << INDENT(depth + 1) << elem->to_string(depth + 2) << "\n";
    }
    return oss.str();
}

std::string ast::ExprTuple::to_string(size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "ExprTuple:\n";
    for (auto& val: vals) {
        oss << INDENT(depth + 1) << val->to_string(depth + 2) << "\n";
    }
    return oss.str();
}

std::string ast::ExprLambda::to_string(size_t depth) const
{
    return INDENT(depth) + "ExprLambda(<lambda>)";
}

std::string ast::StmtVarDecl::to_string(size_t depth) const
{
    return std::format(
        "{}StmtVarDecl(lval={}, rval={}, type={})",
        INDENT(depth),
        lval->to_string(depth + 1),
        rval ? rval->to_string(depth + 1) : "<none>",
        type ? type->to_string(depth + 1) : "<inferred>"
    );
}

std::string ast::StmtScope::to_string(size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "StmtScope:\n";
    depth++;
    for (const Stmt* stmt: stmts)
        oss << stmt->to_string(depth) << "\n";
    depth--;
    return oss.str();
}

std::string ast::StmtIf::to_string(size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "StmtIf:\n";
    depth++;
    for (auto& br: brs) {
        oss << INDENT(depth)
            << "Branch(cond=" << (br.cnd ? br.cnd->to_string(depth + 1) : "<none>")
            << ")\n";
        depth++;
        for (auto* stmt: br.br->stmts)
            oss << stmt->to_string(depth) << "\n";
        depth--;
    }
    depth--;
    return oss.str();
}

std::string ast::StmtFor::to_string(size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "StmtFor(init=" << init->to_string(depth + 1)
        << ", target=" << target->to_string(depth + 1)
        << ", step=" << (step ? step->to_string(depth + 1) : "<none>") << ")\n";
    depth++;
    for (auto* stmt: br->stmts)
        oss << stmt->to_string(depth) << "\n";
    depth--;
    return oss.str();
}

std::string ast::StmtForEach::to_string(size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "StmtForEach(lval=" << lval->to_string(depth + 1)
        << ", iter=" << iter->to_string(depth + 1) << ")\n";
    depth++;
    for (auto* stmt: br->stmts)
        oss << stmt->to_string(depth) << "\n";
    depth--;
    return oss.str();
}

std::string ast::StmtWhile::to_string(size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "StmtWhile(cond=" << cnd->to_string(depth + 1) << ")\n";
    depth++;
    for (auto* stmt: br->stmts)
        oss << stmt->to_string(depth) << "\n";
    depth--;
    return oss.str();
}

std::string ast::StmtAssign::to_string(size_t depth) const
{
    return INDENT(depth) + std::format(
                               "StmtAssign(op={}, lval={}, rval={})",
                               op->to_string(),
                               lval->to_string(depth + 1),
                               rval->to_string(depth + 1)
                           );
}

std::string ast::StmtReturn::to_string(size_t depth) const
{
    return INDENT(depth) +
           std::format("StmtReturn({})", expr ? expr->to_string(depth + 1) : "<null>");
}

std::string ast::TypeBuiltin::to_string(size_t depth) const
{
    return INDENT(depth) + std::format("TypeBuiltin({})", tok->to_string());
}

std::string ast::TypeArray::to_string(size_t depth) const
{
    return INDENT(depth) + std::format("TypeArray({})", type->to_string(depth + 1));
}

std::string ast::TypeDict::to_string(size_t depth) const
{
    return INDENT(depth) + std::format(
                               "TypeDict(key={}, val={})",
                               key->to_string(depth + 1),
                               val->to_string(depth + 1)
                           );
}

std::string ast::TypeFunc::to_string(size_t depth) const
{
    std::ostringstream oss;
    oss << INDENT(depth) << "TypeFunc(ret=" << ret->to_string(depth + 1) << ", parms=[\n";
    for (auto& parm: params)
        oss << parm->to_string() << "\n";
    oss << INDENT(depth) << "])";
    return oss.str();
}

bool ast::is_lvalue(const Expr* expr) noexcept
{
    return TRY_IS(const ExprSymbol, expr) || TRY_IS(const ExprStaticAccess, expr) ||
           TRY_IS(const ExprDynAccess, expr) || TRY_IS(const ExprSubscript, expr) ||
           TRY_IS(const ExprTuple, expr);
}

[[nodiscard]] std::string via::debug::to_string(const via::SyntaxTree& ast)
{
    std::ostringstream oss;
    for (const auto* st: ast)
        oss << st->to_string(0) << "\n";
    return oss.str();
}
