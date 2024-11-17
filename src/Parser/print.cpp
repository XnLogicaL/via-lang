/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "print.h"
#include "ast.h"
#include "token.h"
#include "common.h"

#include "magic_enum.hpp"
#include "Utils/format_vec.h"

using namespace via::Parsing;

std::string AST::stringify_type_node(const TypeNode& type)
{
    if (auto lit_node = std::get_if<LiteralTypeNode>(&type))
    {
        return std::format("LiteralTypeNode(Type: {}, Args: {})",
            stringify_type_node(*lit_node),
            format_vector<TypeNode*>(lit_node->args, [](const TypeNode* t) {
                return stringify_type_node(*t);
            })
        );
    }
    else if (auto union_node = std::get_if<UnionTypeNode>(&type))
    {
        return std::format("UnionTypeNode(L: {}, R: {})",
            stringify_type_node(*union_node->lhs),
            stringify_type_node(*union_node->rhs)
        );
    }
    else if (auto variant_node = std::get_if<VariantTypeNode>(&type))
    {
        return std::format("VariantTypeNode(Variants: {})",
            format_vector<TypeNode*>(variant_node->types, [](const TypeNode* t) {
                return stringify_type_node(*t);
            })
        );
    }
    else if (auto functor_node = std::get_if<FunctorTypeNode>(&type))
    {
        return std::format("FunctorTypeNode(Input: {}, Output: {})",
            format_vector<TypeNode*>(functor_node->input, [](const TypeNode* t) {
                return stringify_type_node(*t);
            }),
            format_vector<TypeNode*>(functor_node->output, [](const TypeNode* t) {
                return stringify_type_node(*t);
            })
        );
    }
    else if (auto table_node = std::get_if<TableTypeNode>(&type))
    {
        return std::format("TableTypeNode(Type: {})",
            stringify_type_node(*table_node->type)
        );
    }
    
    return "UnknownTypeNode()";
}

std::string AST::stringify_expr_node(const ExprNode& expr)
{
    if (auto lit_expr = std::get_if<LitExprNode>(&expr))
    {
        return std::format("LitExprNode(Val: {})",
            lit_expr->val.to_string()
        );
    }
    else if (auto un_expr = std::get_if<UnExprNode>(&expr))
    {
        return std::format("UnExprNode(Expr: {})",
            stringify_expr_node(*un_expr->expr)
        );
    }
    else if (auto group_expr = std::get_if<GroupExprNode>(&expr))
    {
        return std::format("GroupExprNode(Expr: {})",
            stringify_expr_node(*group_expr->expr)
        );
    }
    else if (auto bin_expr = std::get_if<BinExprNode>(&expr))
    {
        return std::format("BinExprNode(Op: {}, L: {}, R: {})",
            bin_expr->op.to_string(),
            stringify_expr_node(*bin_expr->lhs),
            stringify_expr_node(*bin_expr->rhs)
        );
    }
    
    return "UnknownExpr()";
}

std::string AST::stringify_local_decl_stmt_node(const LocalDeclStmtNode node)
{
    return std::format("LocalDeclStmt(Ident: {}, Type: {}, Const: {}, Val: {})",
        node.ident.to_string(),
        stringify_type_node(*node.type),
        std::to_string(node.is_const),
        stringify_expr_node(*node.val)
    );
}

std::string AST::stringify_glob_decl_stmt_node(const GlobDeclStmtNode node)
{
    return std::format("GlobalDeclStmtNode(Ident: {}, Type: {}, Val: {})",
        node.ident.to_string(),
        stringify_type_node(*node.type),
        stringify_expr_node(*node.val)
    );
}

std::string AST::stringify_call_stmt_node(const CallStmtNode node)
{
    return std::format("CallStmtNode(Ident: {}, TypeArgs: {}, Args: {})",
        node.ident.to_string(),
        format_vector<TypeNode*>(node.type_args, [](const TypeNode* t) {
            return stringify_type_node(*t);
        }),
        format_vector<ExprNode*>(node.args, [](const ExprNode* e) {
            return stringify_expr_node(*e);
        })
    );
}

std::string AST::stringify_return_stmt_node(const ReturnStmtNode node)
{
    return std::format("ReturnStmtNode(Vals: {})",
        format_vector<ExprNode*>(node.vals, [](const ExprNode* e) {
            return stringify_expr_node(*e);
        })
    );
}

std::string AST::stringify_scope_stmt_node(const ScopeStmtNode scope)
{
    return std::format("ScopeStmtNode(Stmts: {})",
        format_vector<StmtNode*>(scope.stmts, [](const StmtNode* s) {
            return stringify_stmt_node(*s);
        })
    );
}

std::string AST::stringify_stmt_node(const StmtNode& node)
{
    if (auto local_decl = std::get_if<LocalDeclStmtNode>(&node))
    {
        return stringify_local_decl_stmt_node(*local_decl);
    }
    else if (auto global_decl = std::get_if<GlobDeclStmtNode>(&node))
    {
        return stringify_glob_decl_stmt_node(*global_decl);
    }
    else if (auto call_stmt = std::get_if<CallStmtNode>(&node))
    {
        return stringify_call_stmt_node(*call_stmt);
    }
    else if (auto ret_stmt = std::get_if<ReturnStmtNode>(&node))
    {
        return stringify_return_stmt_node(*ret_stmt);
    }
    else if (auto scope_stmt = std::get_if<ScopeStmtNode>(&node))
    {
        return stringify_scope_stmt_node(*scope_stmt);
    }

    return "UnknownStmt()";
}

std::string AST::stringify_ast(const AST ast)
{
    return std::format("AST(Stmts:{})",
        format_vector<StmtNode*>(ast.stmts, [](const StmtNode* s) {
            return " " + stringify_stmt_node(*s);
        })
    );
}