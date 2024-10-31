#include "print.h"
#include "ast.h"
#include "../Lexer/token.h"
#include "common.h"
#include "magic_enum/magic_enum.hpp"

using namespace via::Parsing;

void AST::print_token(const Token& token)
{
    std::cout << "Token(Type: " << magic_enum::enum_name(token.type) << ", Value: " << token.value << ")";
}

void AST::print_type_node(const TypeNode* type)
{
    std::cout << "TypeNode(Name: " << "some shit idfk" << ")";
}

void AST::print_expr_node(const ExprNode* expr)
{
    std::cout << "SOME FUCKING EXPRESSION";
}

void AST::print_typed_param_stmt_node(const TypedParamStmtNode& node)
{
    std::cout << "TypedParamStmtNode: ";

    print_token(node.ident);

    std::cout << ", ";

    print_type_node(node.type);

    std::cout << std::endl;
}

void AST::print_local_decl_stmt_node(const LocalDeclStmtNode& node)
{
    std::cout << "LocalDeclStmtNode: ";

    print_token(node.ident);

    std::cout << ", ";

    print_type_node(node.type);

    std::cout << ", Value: ";

    if (node.val)
    {
        print_expr_node(node.val);
    }
    else
    {
        std::cout << "null";
    }

    std::cout << ", Is Const: " << (node.is_const ? "true" : "false") << ")" << std::endl;
}

void AST::print_glob_decl_stmt_node(const GlobDeclStmtNode& node)
{
    std::cout << "GlobDeclStmtNode: ";

    print_token(node.ident);

    std::cout << ", ";

    print_type_node(node.type);
    
    std::cout << ", Value: ";

    if (node.val)
    {
        print_expr_node(node.val);
    }
    else
    {
        std::cout << "null";
    }

    std::cout << ")" << std::endl;
}

void AST::print_call_stmt_node(const CallStmtNode& node)
{
    std::cout << "CallStmtNode: ";

    print_token(node.ident);

    std::cout << ", Args: [";

    for (const auto& arg : node.args)
    {
        print_expr_node(arg);
        std::cout << ", ";
    }

    std::cout << "], Type Args: [";

    for (const auto& type_arg : node.type_args)
    {
        print_type_node(type_arg);
        std::cout << ", ";
    }

    std::cout << "])" << std::endl;
}

void AST::print_return_stmt_node(const ReturnStmtNode& node)
{
    std::cout << "ReturnStmtNode: Values: [";

    for (const auto& val : node.vals)
    {
        print_expr_node(val);
        std::cout << ", ";
    }

    std::cout << "]" << std::endl;
}

void AST::print_scope_stmt_node(const ScopeStmtNode& scope)
{
    std::cout << "ScopeStmtNode: Statements: [" << std::endl;

    for (const auto& stmt : scope.stmts)
    {
        print_stmt_node(stmt);
    }

    std::cout << "]" << std::endl;
}

void AST::print_stmt_node(StmtNode* node)
{
    if (auto local_decl = std::get_if<LocalDeclStmtNode>(node))
    {
        print_local_decl_stmt_node(*local_decl);
    }
    else if (auto global_decl = std::get_if<GlobDeclStmtNode>(node))
    {
        print_glob_decl_stmt_node(*global_decl);
    }
    else if (auto call_stmt = std::get_if<CallStmtNode>(node))
    {
        print_call_stmt_node(*call_stmt);
    }
    else if (auto ret_stmt = std::get_if<ReturnStmtNode>(node))
    {
        print_return_stmt_node(*ret_stmt);
    }
    else if (auto scope_stmt = std::get_if<ScopeStmtNode>(node))
    {
        print_scope_stmt_node(*scope_stmt);
    }
}

void AST::print_ast(AST* ast)
{
    for (const auto &stmt : ast->stmts)
    {
        print_stmt_node(stmt);
    }
}