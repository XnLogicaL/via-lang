/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "inline.h"
#include "Parser/ast.h"

using namespace via::Parsing;

namespace via::Compilation
{

AST::IdentExprNode *_inlineable(AST::ExprNode *expr)
{
    return std::get_if<AST::IdentExprNode>(expr);
}

std::vector<AST::IdentExprNode *> _get_inlineable_exprs(AST::AST *ast)
{
    std::vector<AST::IdentExprNode *> exprs;

    for (auto *stmt : ast->stmts)
    {
        if (auto local_decl_stmt = std::get_if<AST::LocalDeclStmtNode>(stmt))
        {
            if (auto ident_expr = _inlineable(local_decl_stmt->val))
                exprs.push_back(ident_expr);
            continue;
        }
        else if (auto global_decl_stmt = std::get_if<AST::GlobDeclStmtNode>(stmt))
        {
            if (auto ident_expr = _inlineable(global_decl_stmt->val))
                exprs.push_back(ident_expr);
            continue;
        }
    }

    return exprs;
}

void optimize_inline_const(AST::AST *ast, std::string id, AST::ExprNode expr)
{
    auto inlineables = _get_inlineable_exprs(ast);

    for (AST::IdentExprNode *inlineable : inlineables)
    {
        if (inlineable->val.value == id)
            // Yes, this is a hack
            *reinterpret_cast<AST::ExprNode *>(inlineable) = expr;
    }
}

} // namespace via::Compilation