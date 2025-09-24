/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "ast.h"
#include "debug.h"

namespace ast = via::ast;

inline size_t ZERO = 0;

#define INDENT std::string(depth, ' ')

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

std::string ast::AttributeGroup::to_string() const
{
    return std::format("AttributeGroup({})", debug::to_string(ats, [](const auto& atr) {
                           return std::format(
                               "Attribute(sp={}, args={})",
                               atr.sp->to_string(),
                               debug::to_string(
                                   atr.args,
                                   [](const auto& arg) { return arg->to_string(); }
                               )
                           );
                       }));
}

std::string ast::ExprLiteral::to_string(size_t&) const
{
    return std::format("ExprLiteral({})", tok->to_string());
}

std::string ast::ExprSymbol::to_string(size_t&) const
{
    return std::format("ExprSymbol({})", symbol->to_string());
}

std::string ast::ExprDynAccess::to_string(size_t&) const
{
    return std::format(
        "ExprDynAccess({}, {})",
        root->to_string(ZERO),
        index->to_string()
    );
}

std::string ast::ExprStaticAccess::to_string(size_t&) const
{
    return std::format(
        "ExprStaticAccess({}, {})",
        root->to_string(ZERO),
        index->to_string()
    );
}

std::string ast::ExprUnary::to_string(size_t&) const
{
    return std::format("ExprUnary({}, {})", op->to_string(), expr->to_string(ZERO));
}

std::string ast::ExprBinary::to_string(size_t&) const
{
    return std::format(
        "ExprBinary({}, {}, {})",
        op->to_string(),
        lhs->to_string(ZERO),
        rhs->to_string(ZERO)
    );
}

std::string ast::ExprGroup::to_string(size_t&) const
{
    return std::format("ExprGroup({})", expr->to_string(ZERO));
}

std::string ast::ExprCall::to_string(size_t&) const
{
    return std::format(
        "ExprCall(callee={}, args={})",
        lval->to_string(ZERO),
        debug::to_string(args, [](const auto& arg) { return arg->to_string(ZERO); })
    );
}

std::string ast::ExprSubscript::to_string(size_t&) const
{
    return std::format(
        "ExprSubscript({}, {})",
        lval->to_string(ZERO),
        idx->to_string(ZERO)
    );
}

std::string ast::ExprCast::to_string(size_t&) const
{
    return std::format("ExprCast({}, {})", expr->to_string(ZERO), type->to_string(ZERO));
}

std::string ast::ExprTernary::to_string(size_t&) const
{
    return std::format(
        "ExprTernary(cnd={}, lhs={}, rhs={})",
        cnd->to_string(ZERO),
        lhs->to_string(ZERO),
        rhs->to_string(ZERO)
    );
}

std::string ast::ExprArray::to_string(size_t&) const
{
    return std::format("ExprArray(init={})", debug::to_string(init, [](const auto& ini) {
                           return ini->to_string(ZERO);
                       }));
}

std::string ast::ExprTuple::to_string(size_t&) const
{
    return std::format("ExprTuple(vals={})", debug::to_string(vals, [](const auto& ini) {
                           return ini->to_string(ZERO);
                       }));
}

std::string ast::ExprLambda::to_string(size_t&) const
{
    return std::format("ExprLambda(<lambda>)");
}

std::string ast::StmtVarDecl::to_string(size_t& depth) const
{
    return INDENT + std::format(
                        "StmtVarDecl(lval={}, rval={}, type={})",
                        lval->to_string(ZERO),
                        rval->to_string(ZERO),
                        type ? type->to_string(ZERO) : "<infered>"
                    );
}

std::string ast::StmtScope::to_string(size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT << "StmtScope()\n";
    depth++;

    for (const Stmt* stmt: stmts) {
        oss << stmt->to_string(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndScope()";
    return oss.str();
}

std::string ast::StmtIf::to_string(size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT << "StmtIf()\n";
    depth++;

    for (const auto& br: brs) {
        oss << INDENT
            << std::format("Branch({})\n", br.cnd ? br.cnd->to_string(ZERO) : "");
        depth++;

        for (const Stmt* stmt: br.br->stmts) {
            oss << stmt->to_string(depth) << "\n";
        }

        depth--;
        oss << INDENT << "EndBranch()\n";
    }

    depth--;
    oss << INDENT << "EndIf()";
    return oss.str();
}

std::string ast::StmtFor::to_string(size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT
        << std::format(
               "StmtFor(init={}, target={}, step={})\n",
               init->to_string(ZERO),
               target->to_string(ZERO),
               step ? step->to_string(ZERO) : "<infered>"
           );
    depth++;

    for (const Stmt* stmt: br->stmts) {
        oss << stmt->to_string(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndFor()";
    return oss.str();
}

std::string ast::StmtForEach::to_string(size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT
        << std::format(
               "StmtForEach(lval={}, iter={})\n",
               lval->to_string(ZERO),
               iter->to_string(ZERO)
           );
    depth++;

    for (const Stmt* stmt: br->stmts) {
        oss << stmt->to_string(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndForEach()";
    return oss.str();
}

std::string ast::StmtWhile::to_string(size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT << std::format("StmtWhile(cnd={})\n", cnd->to_string(ZERO));
    depth++;

    for (const Stmt* stmt: br->stmts) {
        oss << stmt->to_string(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndWhile()";
    return oss.str();
}

std::string ast::StmtAssign::to_string(size_t& depth) const
{
    return INDENT + std::format(
                        "StmtAssign(op={}, lval={}, rval={})",
                        op->to_string(),
                        lval->to_string(ZERO),
                        rval->to_string(ZERO)
                    );
}

std::string ast::StmtReturn::to_string(size_t& depth) const
{
    return INDENT +
           std::format("StmtReturn({})", expr ? expr->to_string(ZERO) : "<null>");
}

std::string ast::StmtEnum::to_string(size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT
        << std::format(
               "StmtEnum(symbol={}, type={})\n",
               symbol->to_string(),
               type->to_string(ZERO)
           );
    depth++;

    for (const auto& entry: pairs) {
        oss << std::format(
            "EnumEntry(symbol={}, expr={})\n",
            entry.symbol->to_string(),
            entry.expr->to_string(ZERO)
        );
    }

    depth--;
    oss << INDENT << "EndEnum()";
    return oss.str();
}

std::string ast::StmtModule::to_string(size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT << std::format("StmtModule({})\n", symbol->to_string());
    depth++;

    for (const auto& stmt: scope) {
        oss << stmt->to_string(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndModule()";
    return oss.str();
}

std::string ast::StmtImport::to_string(size_t& depth) const
{
    return INDENT +
           std::format("StmtImport({})", debug::to_string(path, [](const auto& node) {
                           return node->to_string();
                       }));
}

std::string ast::StmtFunctionDecl::to_string(size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT + std::format(
                        "StmtFunctionDecl(name={}, ret={}, parms={})\n",
                        name->to_string(),
                        ret->to_string(ZERO),
                        debug::to_string(
                            parms,
                            [](const auto& parm) { return parm->to_string(); }
                        )
                    );
    depth++;

    for (const auto& stmt: scope->stmts) {
        oss << stmt->to_string(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndFunctionDecl()";
    return oss.str();
}

std::string ast::StmtStructDecl::to_string(size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT + std::format("StmtStructDecl({})\n", name->to_string());
    depth++;

    for (const auto& stmt: scope) {
        oss << stmt->to_string(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndStructDecl()";
    return oss.str();
}

std::string ast::StmtTypeDecl::to_string(size_t& depth) const
{
    return INDENT + std::format(
                        "StmtTypeDecl(symbol={}, type={})",
                        symbol->to_string(),
                        type->to_string(ZERO)
                    );
}

std::string ast::StmtUsing::to_string(size_t& depth) const
{
    std::ostringstream oss;
    oss << INDENT + std::format("StmtUsing({})\n", sp->to_string());
    depth++;

    for (const auto& stmt: scope->stmts) {
        oss << stmt->to_string(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndUsing()";
    return oss.str();
}

std::string ast::StmtEmpty::to_string(size_t& depth) const
{
    return INDENT + "StmtEmpty()";
}

std::string ast::StmtExpr::to_string(size_t& depth) const
{
    return INDENT + std::format("StmtExpr({})", expr->to_string(ZERO));
}

std::string ast::TypeBuiltin::to_string(size_t&) const
{
    return std::format("TypeBuiltin({})", tok->to_string());
}

std::string ast::TypeArray::to_string(size_t&) const
{
    return std::format("TypeArray({})", type->to_string(ZERO));
}

std::string ast::TypeDict::to_string(size_t&) const
{
    return std::format(
        "TypeDict(key={}, val={})",
        key->to_string(ZERO),
        val->to_string(ZERO)
    );
}

std::string ast::TypeFunc::to_string(size_t&) const
{
    return std::format(
        "TypeFunc(ret={}, parms={})",
        ret->to_string(ZERO),
        debug::to_string(params, [](const auto& parm) { return parm->to_string(); })
    );
}

bool ast::isLValue(const Expr* expr) noexcept
{
    return TRY_IS(const ExprSymbol, expr) || TRY_IS(const ExprStaticAccess, expr) ||
           TRY_IS(const ExprDynAccess, expr) || TRY_IS(const ExprSubscript, expr) ||
           TRY_IS(const ExprTuple, expr);
}

[[nodiscard]] std::string via::debug::to_string(const via::SyntaxTree& ast)
{
    std::ostringstream oss;
    size_t depth = 0;

    for (const auto* st: ast) {
        oss << st->to_string(depth) << "\n";
    }

    return oss.str();
}
