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

inline via::usize ZERO = 0;

#define INDENT std::string(depth, ' ')

std::string ast::Path::get_dump() const
{
    return std::format("Path({})", debug::get_dump(path, [](const auto& node) {
                           return node->get_dump();
                       }));
}

std::string ast::Parameter::get_dump() const
{
    return std::format(
        "Parameter(symbol={}, type={})",
        symbol->get_dump(),
        type->get_dump(ZERO)
    );
}

std::string ast::AttributeGroup::get_dump() const
{
    return std::format("AttributeGroup({})", debug::get_dump(ats, [](const auto& atr) {
                           return std::format(
                               "Attribute(sp={}, args={})",
                               atr.sp->get_dump(),
                               debug::get_dump(
                                   atr.args,
                                   [](const auto& arg) { return arg->get_dump(); }
                               )
                           );
                       }));
}

std::string ast::ExprLiteral::get_dump(usize&) const
{
    return std::format("ExprLiteral({})", tok->get_dump());
}

std::string ast::ExprSymbol::get_dump(usize&) const
{
    return std::format("ExprSymbol({})", symbol->get_dump());
}

std::string ast::ExprDynAccess::get_dump(usize&) const
{
    return std::format("ExprDynAccess({}, {})", root->get_dump(ZERO), index->to_string());
}

std::string ast::ExprStaticAccess::get_dump(usize&) const
{
    return std::format(
        "ExprStaticAccess({}, {})",
        root->get_dump(ZERO),
        index->to_string()
    );
}

std::string ast::ExprUnary::get_dump(usize&) const
{
    return std::format("ExprUnary({}, {})", op->get_dump(), expr->get_dump(ZERO));
}

std::string ast::ExprBinary::get_dump(usize&) const
{
    return std::format(
        "ExprBinary({}, {}, {})",
        op->get_dump(),
        lhs->get_dump(ZERO),
        rhs->get_dump(ZERO)
    );
}

std::string ast::ExprGroup::get_dump(usize&) const
{
    return std::format("ExprGroup({})", expr->get_dump(ZERO));
}

std::string ast::ExprCall::get_dump(usize&) const
{
    return std::format(
        "ExprCall(callee={}, args={})",
        lval->get_dump(ZERO),
        debug::get_dump(args, [](const auto& arg) { return arg->get_dump(ZERO); })
    );
}

std::string ast::ExprSubscript::get_dump(usize&) const
{
    return std::format(
        "ExprSubscript({}, {})",
        lval->get_dump(ZERO),
        idx->get_dump(ZERO)
    );
}

std::string ast::ExprCast::get_dump(usize&) const
{
    return std::format("ExprCast({}, {})", expr->get_dump(ZERO), type->get_dump(ZERO));
}

std::string ast::ExprTernary::get_dump(usize&) const
{
    return std::format(
        "ExprTernary(cnd={}, lhs={}, rhs={})",
        cnd->get_dump(ZERO),
        lhs->get_dump(ZERO),
        rhs->get_dump(ZERO)
    );
}

std::string ast::ExprArray::get_dump(usize&) const
{
    return std::format("ExprArray(init={})", debug::get_dump(init, [](const auto& ini) {
                           return ini->get_dump(ZERO);
                       }));
}

std::string ast::ExprTuple::get_dump(usize&) const
{
    return std::format("ExprTuple(vals={})", debug::get_dump(vals, [](const auto& ini) {
                           return ini->get_dump(ZERO);
                       }));
}

std::string ast::ExprLambda::get_dump(usize&) const
{
    return std::format("ExprLambda(<lambda>)");
}

std::string ast::StmtVarDecl::get_dump(usize& depth) const
{
    return INDENT + std::format(
                        "StmtVarDecl(lval={}, rval={}, type={})",
                        lval->get_dump(ZERO),
                        rval->get_dump(ZERO),
                        type ? type->get_dump(ZERO) : "<infered>"
                    );
}

std::string ast::StmtScope::get_dump(usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT << "StmtScope()\n";
    depth++;

    for (const Stmt* stmt: stmts) {
        oss << stmt->get_dump(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndScope()";
    return oss.str();
}

std::string ast::StmtIf::get_dump(usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT << "StmtIf()\n";
    depth++;

    for (const auto& br: brs) {
        oss << INDENT
            << std::format("Branch({})\n", br.cnd ? br.cnd->get_dump(ZERO) : "");
        depth++;

        for (const Stmt* stmt: br.br->stmts) {
            oss << stmt->get_dump(depth) << "\n";
        }

        depth--;
        oss << INDENT << "EndBranch()\n";
    }

    depth--;
    oss << INDENT << "EndIf()";
    return oss.str();
}

std::string ast::StmtFor::get_dump(usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT
        << std::format(
               "StmtFor(init={}, target={}, step={})\n",
               init->get_dump(ZERO),
               target->get_dump(ZERO),
               step ? step->get_dump(ZERO) : "<infered>"
           );
    depth++;

    for (const Stmt* stmt: br->stmts) {
        oss << stmt->get_dump(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndFor()";
    return oss.str();
}

std::string ast::StmtForEach::get_dump(usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT
        << std::format(
               "StmtForEach(lval={}, iter={})\n",
               lval->get_dump(ZERO),
               iter->get_dump(ZERO)
           );
    depth++;

    for (const Stmt* stmt: br->stmts) {
        oss << stmt->get_dump(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndForEach()";
    return oss.str();
}

std::string ast::StmtWhile::get_dump(usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT << std::format("StmtWhile(cnd={})\n", cnd->get_dump(ZERO));
    depth++;

    for (const Stmt* stmt: br->stmts) {
        oss << stmt->get_dump(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndWhile()";
    return oss.str();
}

std::string ast::StmtAssign::get_dump(usize& depth) const
{
    return INDENT + std::format(
                        "StmtAssign(op={}, lval={}, rval={})",
                        op->get_dump(),
                        lval->get_dump(ZERO),
                        rval->get_dump(ZERO)
                    );
}

std::string ast::StmtReturn::get_dump(usize& depth) const
{
    return INDENT + std::format("StmtReturn({})", expr ? expr->get_dump(ZERO) : "<null>");
}

std::string ast::StmtEnum::get_dump(usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT
        << std::format(
               "StmtEnum(symbol={}, type={})\n",
               symbol->get_dump(),
               type->get_dump(ZERO)
           );
    depth++;

    for (const auto& entry: pairs) {
        oss << std::format(
            "EnumEntry(symbol={}, expr={})\n",
            entry.symbol->get_dump(),
            entry.expr->get_dump(ZERO)
        );
    }

    depth--;
    oss << INDENT << "EndEnum()";
    return oss.str();
}

std::string ast::StmtModule::get_dump(usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT << std::format("StmtModule({})\n", symbol->get_dump());
    depth++;

    for (const auto& stmt: scope) {
        oss << stmt->get_dump(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndModule()";
    return oss.str();
}

std::string ast::StmtImport::get_dump(usize& depth) const
{
    return INDENT +
           std::format("StmtImport({})", debug::get_dump(path, [](const auto& node) {
                           return node->get_dump();
                       }));
}

std::string ast::StmtFunctionDecl::get_dump(usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT + std::format(
                        "StmtFunctionDecl(name={}, ret={}, parms={})\n",
                        name->get_dump(),
                        ret->get_dump(ZERO),
                        debug::get_dump(
                            parms,
                            [](const auto& parm) { return parm->get_dump(); }
                        )
                    );
    depth++;

    for (const auto& stmt: scope->stmts) {
        oss << stmt->get_dump(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndFunctionDecl()";
    return oss.str();
}

std::string ast::StmtStructDecl::get_dump(usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT + std::format("StmtStructDecl({})\n", name->get_dump());
    depth++;

    for (const auto& stmt: scope) {
        oss << stmt->get_dump(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndStructDecl()";
    return oss.str();
}

std::string ast::StmtTypeDecl::get_dump(usize& depth) const
{
    return INDENT + std::format(
                        "StmtTypeDecl(symbol={}, type={})",
                        symbol->get_dump(),
                        type->get_dump(ZERO)
                    );
}

std::string ast::StmtUsing::get_dump(usize& depth) const
{
    std::ostringstream oss;
    oss << INDENT + std::format("StmtUsing({})\n", sp->get_dump());
    depth++;

    for (const auto& stmt: scope->stmts) {
        oss << stmt->get_dump(depth) << "\n";
    }

    depth--;
    oss << INDENT << "EndUsing()";
    return oss.str();
}

std::string ast::StmtEmpty::get_dump(usize& depth) const
{
    return INDENT + "StmtEmpty()";
}

std::string ast::StmtExpr::get_dump(usize& depth) const
{
    return INDENT + std::format("StmtExpr({})", expr->get_dump(ZERO));
}

std::string ast::TypeBuiltin::get_dump(usize&) const
{
    return std::format("TypeBuiltin({})", tok->get_dump());
}

std::string ast::TypeArray::get_dump(usize&) const
{
    return std::format("TypeArray({})", type->get_dump(ZERO));
}

std::string ast::TypeDict::get_dump(usize&) const
{
    return std::format(
        "TypeDict(key={}, val={})",
        key->get_dump(ZERO),
        val->get_dump(ZERO)
    );
}

std::string ast::TypeFunc::get_dump(usize&) const
{
    return std::format(
        "TypeFunc(ret={}, parms={})",
        ret->get_dump(ZERO),
        debug::get_dump(params, [](const auto& parm) { return parm->get_dump(); })
    );
}

bool ast::isLValue(const Expr* expr) noexcept
{
    return TRY_IS(const ExprSymbol, expr) || TRY_IS(const ExprStaticAccess, expr) ||
           TRY_IS(const ExprDynAccess, expr) || TRY_IS(const ExprSubscript, expr) ||
           TRY_IS(const ExprTuple, expr);
}

[[nodiscard]] std::string via::debug::get_dump(const via::SyntaxTree& ast)
{
    std::ostringstream oss;
    usize depth = 0;

    for (const auto* st: ast) {
        oss << st->get_dump(depth) << "\n";
    }

    return oss.str();
}
