/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "ast.hpp"
#include <sstream>
#include "support/ansi.hpp"
#include "support/utility.hpp"

#define INDENT(depth) std::string(depth * 2, ' ')

std::string via::ast::Path::to_string() const
{
    return via::to_string(
        path,
        [](const auto& node) { return node->to_string(); },
        "",
        "",
        "::"
    );
}

std::string via::ast::Parameter::to_string() const
{
    return std::format("{}: {}", symbol->to_string(), type->to_string());
}

std::string via::ast::Scope::to_string(size_t depth) const
{
    return via::to_string(
        stmts,
        [depth](const auto& arg) { return arg->to_string(depth + 1); },
        "{\n",
        INDENT(depth) + "}",
        ";\n"
    );
}

std::string via::ast::ExprLiteral::to_string(size_t) const
{
    return tok->to_string();
}

std::string via::ast::ExprSymbol::to_string(size_t) const
{
    return symbol->to_string();
}

std::string via::ast::ExprDynAccess::to_string(size_t depth) const
{
    return std::format("{}.{}", root->to_string(), index->to_string());
}

std::string via::ast::ExprStaticAccess::to_string(size_t depth) const
{
    return std::format("{}::{}", root->to_string(), index->to_string());
}

std::string via::ast::ExprUnary::to_string(size_t depth) const
{
    return std::format("({}{})", op->to_string(), expr->to_string());
}

std::string via::ast::ExprBinary::to_string(size_t depth) const
{
    return std::format("({}{}{})", lhs->to_string(), op->to_string(), rhs->to_string());
}

std::string via::ast::ExprGroup::to_string(size_t depth) const
{
    return std::format("({})", expr->to_string());
}

std::string via::ast::ExprCall::to_string(size_t depth) const
{
    return std::format(
        "{}{}",
        callee->to_string(),
        via::to_string(args, [](const auto& arg) { return arg->to_string(); }, "(", ")")
    );
}

std::string via::ast::ExprSubscript::to_string(size_t depth) const
{
    return std::format("{}[{}]", lhs->to_string(), rhs->to_string());
}

std::string via::ast::ExprCast::to_string(size_t depth) const
{
    return std::format("{} as {}", expr->to_string(), type->to_string());
}

std::string via::ast::ExprTernary::to_string(size_t depth) const
{
    return std::format(
        "({} if {} else {})",
        lhs->to_string(),
        cond->to_string(),
        rhs->to_string()
    );
}

std::string via::ast::ExprArray::to_string(size_t depth) const
{
    return via::to_string(values, [](const auto& arg) { return arg->to_string(); });
}

std::string via::ast::ExprTuple::to_string(size_t depth) const
{
    return via::to_string(
        values,
        [](const auto& arg) { return arg->to_string(); },
        "(",
        ")"
    );
}

std::string via::ast::ExprLambda::to_string(size_t depth) const
{
    return std::format(
        "fn {} -> {} {}",
        via::to_string(
            parms,
            [](const auto& arg) { return arg->to_string(); },
            "(",
            ")"
        ),
        ret != nullptr ? ret->to_string() : "<infered>",
        body->to_string(depth)
    );
}

std::string via::ast::StmtVarDecl::to_string(size_t depth) const
{
    return INDENT(depth) + std::format(
                               "var {}: {} = {}",
                               lval->to_string(),
                               type ? type->to_string() : "<infered>",
                               rval ? rval->to_string() : "<none>"
                           );
}

std::string via::ast::StmtScope::to_string(size_t depth) const
{
    return INDENT(depth) + "do " + body->to_string(depth);
}

std::string via::ast::StmtIf::to_string(size_t depth) const
{
    std::ostringstream oss;
    for (size_t i = 0; auto& branch: branches) {
        oss << INDENT(depth);
        if (i == 0)
            oss << "if " << branch.cond->to_string();
        else if (branch.cond == nullptr)
            oss << "else ";
        else
            oss << "else if " << branch.cond->to_string();
        oss << branch.body->to_string(depth + 1) << "\n";
    }
    return oss.str();
}

std::string via::ast::StmtFor::to_string(size_t depth) const
{
    return INDENT(depth) + std::format(
                               "for {}, {}, {} {}",
                               init->to_string(),
                               target->to_string(),
                               step != nullptr ? step->to_string() : "<infered>",
                               body->to_string(depth)
                           );
}

std::string via::ast::StmtForEach::to_string(size_t depth) const
{
    return INDENT(depth) + std::format(
                               "for {} in {} {}",
                               name->to_string(),
                               expr->to_string(),
                               body->to_string(depth)
                           );
}

std::string via::ast::StmtWhile::to_string(size_t depth) const
{
    return INDENT(depth) +
           std::format("while {} {}", cond->to_string(), body->to_string(depth));
}

std::string via::ast::StmtAssign::to_string(size_t depth) const
{
    return INDENT(depth) + std::format(
                               "{} {}= {}",
                               lval->to_string(),
                               op->to_string(),
                               rval->to_string()
                           );
}

std::string via::ast::StmtReturn::to_string(size_t depth) const
{
    return INDENT(depth) + std::format("return {}", expr ? expr->to_string() : "<null>");
}

std::string via::ast::StmtEnum::to_string(size_t depth) const
{
    return INDENT(depth) + std::format(
                               "enum {} of {} {}",
                               symbol->to_string(),
                               type != nullptr ? type->to_string() : "<infered>",
                               via::to_string(
                                   pairs,
                                   [depth](const auto& arg) {
                                       return INDENT(depth) + std::format(
                                                                  "{} = {}",
                                                                  arg.symbol->to_string(),
                                                                  arg.expr->to_string()
                                                              );
                                   },
                                   "{\n",
                                   INDENT(depth) + "}",
                                   ",\n"
                               )
                           );
}

std::string via::ast::StmtImport::to_string(size_t depth) const
{
    return INDENT(depth) + std::format(
                               "import {}",
                               via::to_string(
                                   path,
                                   [](const auto& arg) { return arg->to_string(); },
                                   "",
                                   "",
                                   "::"
                               )
                           );
}

std::string via::ast::StmtFunctionDecl::to_string(size_t depth) const
{
    return INDENT(depth) + std::format(
                               "fn {}{} -> {} {}",
                               name->to_string(),
                               via::to_string(
                                   parms,
                                   [](const auto& arg) { return arg->to_string(); },
                                   "(",
                                   ")"
                               ),
                               ret != nullptr ? ret->to_string() : "<infered>",
                               body->to_string(depth)
                           );
}

std::string via::ast::StmtStructDecl::to_string(size_t depth) const
{
    return INDENT(depth) +
           std::format("struct {} {}", name->to_string(), body->to_string(depth));
}

std::string via::ast::StmtTypeDecl::to_string(size_t depth) const
{
    return INDENT(depth) +
           std::format("type {} = {}", symbol->to_string(), type->to_string());
}

std::string via::ast::StmtEmpty::to_string(size_t depth) const
{
    return INDENT(depth);
}

std::string via::ast::StmtExpr::to_string(size_t depth) const
{
    return INDENT(depth) + expr->to_string();
}

std::string via::ast::TypeBuiltin::to_string(size_t) const
{
    return token->to_string();
}

std::string via::ast::TypeArray::to_string(size_t) const
{
    return std::format("[{}]", type->to_string());
}

std::string via::ast::TypeMap::to_string(size_t) const
{
    return std::format("{{{}: {}}}", key->to_string(), value->to_string());
}

std::string via::ast::TypeFunc::to_string(size_t) const
{
    return std::format(
        "fn {} -> {}",
        via::to_string(
            parms,
            [](const auto& arg) { return arg->to_string(); },
            "(",
            ")"
        ),
        ret->to_string()
    );
}

bool via::ast::is_lvalue(const Expr* expr) noexcept
{
    return TRY_IS(const ExprSymbol, expr) || TRY_IS(const ExprStaticAccess, expr) ||
           TRY_IS(const ExprDynAccess, expr) || TRY_IS(const ExprSubscript, expr) ||
           TRY_IS(const ExprTuple, expr);
}

std::string via::to_string(const via::SyntaxTree& tree)
{
    std::ostringstream oss;
    oss << via::ansi::format(
        "[disassembly of program AST]:\n",
        ansi::Foreground::YELLOW,
        ansi::Background::NONE,
        ansi::Style::UNDERLINE
    );

    for (const auto& node: tree)
        oss << node->to_string(1) << ";\n";
    return oss.str();
}
