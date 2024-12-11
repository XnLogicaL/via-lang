/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "inline.h"
#include "Parser/ast.h"

using namespace via::Parsing;

namespace via::Compilation
{

AST::VarExprNode *_inlineable(AST::ExprNode *expr)
{
    return std::get_if<AST::VarExprNode>(expr);
}

std::vector<AST::VarExprNode *> _get_inlineable_exprs(AST::AST *ast)
{
    std::vector<AST::VarExprNode *> exprs;

    for (auto stmt : ast->statements)
    {
        if (auto local_decl_stmt = std::get_if<AST::LocalDeclStmtNode>(&stmt))
        {
            if (auto ident_expr = _inlineable(&local_decl_stmt->value.value()))
                exprs.push_back(ident_expr);
            else if (auto global_decl_stmt = std::get_if<AST::GlobalDeclStmtNode>(&stmt))
                if (auto ident_expr = _inlineable(&global_decl_stmt->value.value()))
                    exprs.push_back(ident_expr);
        }

        continue;
    }

    return exprs;
}

void optimize_inline_const(AST::AST *ast, std::string id, AST::ExprNode expr)
{
    auto inlineables = _get_inlineable_exprs(ast);

    for (AST::VarExprNode *inlineable : inlineables)
    {
        if (inlineable->ident.value == id)
            // Yes, this is a hack
            *reinterpret_cast<AST::ExprNode *>(inlineable) = expr;
    }
}

} // namespace via::Compilation