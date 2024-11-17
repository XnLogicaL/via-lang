/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "common.h"
#include "parser.h"
#include "ast.h"

#define CAST_VARIANT_POINTER(type, p) \
    m_alloc.emplace<type>(*p)

using namespace via;
using namespace Tokenization;
using namespace Parsing;

/* Utilities */
Token Parser::consume()
{
    return container.tokens.at(current_position++);
}

Token Parser::peek(int offset) const
{
    return container.tokens.at(current_position + offset);
}

bool Parser::is_value(const std::string& value, int offset) const
{
    return peek(offset).value == value;
}

bool Parser::is_type(Tokenization::TokenType type, int offset) const
{
    return peek(offset).type == type;
}

/* Helper functions */
std::vector<AST::ExprNode*> Parser::parse_call_arguments()
{
    consume();

    std::vector<AST::ExprNode*> args;
    bool expecting_arg = true;

    while (!is_type(TokenType::PAREN_CLOSE))
    {
        if (expecting_arg)
        {
            args.push_back(parse_expr());
        }
        else
        {
            consume();
        }

        expecting_arg = !expecting_arg;
    }

    consume();

    return args;
}

std::vector<AST::TypeNode*> Parser::parse_call_type_arguments()
{
    consume();

    std::vector<AST::TypeNode*> types;
    bool expecting_type = true;

    while (!is_type(TokenType::OP_GT))
    {
        if (expecting_type)
        {
            types.push_back(parse_type());
        }
        else
        {
            consume();
        }

        expecting_type = !expecting_type;
    }

    consume();

    return types;    
}

template <typename IndexExprType, typename IndexCallExprType>
AST::ExprNode* Parser::parse_index_expr(AST::ExprNode* base_expr)
{
    auto index = parse_expr();
    consume(); // Consume closing bracket or parenthesis

    if (is_type(TokenType::OP_LT) || is_type(TokenType::PAREN_OPEN))
    {
        auto index_call_expr = m_alloc.emplace<IndexCallExprType>();
        index_call_expr->ident = CAST_VARIANT_POINTER(AST::ExprNode, base_expr);
        index_call_expr->index = index;

        if (is_type(TokenType::OP_LT))
        {
            index_call_expr->type_args = parse_call_type_arguments();
        }

        index_call_expr->args = parse_call_arguments();
        return CAST_VARIANT_POINTER(AST::ExprNode, index_call_expr);
    }
    else
    {
        auto index_expr = m_alloc.emplace<IndexExprType>();
        index_expr->ident = CAST_VARIANT_POINTER(AST::ExprNode, base_expr);
        index_expr->index = index;

        return CAST_VARIANT_POINTER(AST::ExprNode, index_expr);
    }
}

AST::ExprNode* Parser::parse_literal_or_group_expr(Token current)
{
    if (current.is_literal())
    {
        auto lit_expr = m_alloc.emplace<AST::LitExprNode>();
        lit_expr->val = current;

        return CAST_VARIANT_POINTER(AST::ExprNode, lit_expr);
    }
    else if (current.type == TokenType::PAREN_OPEN)
    {
        auto expr = parse_expr();
        consume(); // Consume closing parenthesis

        auto group_expr = m_alloc.emplace<AST::GroupExprNode>();
        group_expr->expr = expr;

        return CAST_VARIANT_POINTER(AST::ExprNode, group_expr);
    }

    return nullptr;
}

/* Main functions */
AST::ExprNode* Parser::parse_prim_expr()
{
    auto current = consume();

    // Literal or grouped expression
    if (auto expr = parse_literal_or_group_expr(current))
    {
        return expr;
    }

    // Unary expression
    if (current.type == TokenType::OP_SUB)
    {
        auto un_expr = m_alloc.emplace<AST::UnExprNode>();
        un_expr->expr = parse_expr();

        return CAST_VARIANT_POINTER(AST::ExprNode, un_expr);
    }

    // Identifier and dot/bracket indexing
    if (current.type == TokenType::IDENTIFIER || current.type == TokenType::PAREN_OPEN)
    {
        auto base_expr = CAST_VARIANT_POINTER(AST::ExprNode, m_alloc.emplace<AST::IdentExprNode>(current));

        if (is_type(TokenType::DOT))
        {
            consume(); // Consume the dot
            return parse_index_expr<AST::IndexExprNode, AST::IndexCallExprNode>(base_expr);
        }
        else if (is_type(TokenType::BRACKET_OPEN))
        {
            consume(); // Consume the bracket
            return parse_index_expr<AST::BracketIndexExprNode, AST::BracketIndexCallExprNode>(base_expr);
        }
    }

    return nullptr;
}

/* Main functions for type parsing */
AST::TypeNode* Parser::parse_type()
{
    Token current = consume();

    if (current.type == TokenType::BRACE_OPEN)
    {
        auto table_type = m_alloc.emplace<AST::TableTypeNode>();
        table_type->type = parse_type();

        consume(); // Consume closing brace

        return CAST_VARIANT_POINTER(AST::TypeNode, table_type);
    }

    if (current.type == TokenType::PAREN_OPEN)
    {
        auto functor_type = m_alloc.emplace<AST::FunctorTypeNode>();

        // Parse input types
        while (!is_type(TokenType::PAREN_CLOSE))
        {
            if (consume().type != TokenType::COMMA)
            {
                functor_type->input.push_back(parse_type());
            }
        }

        consume(); // Consume closing parenthesis for inputs

        consume(); // Consume "->" sequence
        consume(); // Consume "->" sequence

        // Parse output types
        if (is_type(TokenType::PAREN_OPEN))
        {
            while (!is_type(TokenType::PAREN_CLOSE))
            {
                if (consume().type != TokenType::COMMA)
                {
                    functor_type->output.push_back(parse_type());
                }
            }

            consume(); // Consume closing parenthesis for outputs
        }
        else
        {
            functor_type->output.push_back(parse_type());
        }

        return CAST_VARIANT_POINTER(AST::TypeNode, functor_type);
    }

    // Other types (Literal, Union, Variant, Generic)
    auto next_type = m_alloc.emplace<AST::LiteralTypeNode>();
    next_type->type = current;

    if (is_type(TokenType::AMPERSAND))
    {
        consume();

        auto union_type = m_alloc.emplace<AST::UnionTypeNode>();
        union_type->lhs = CAST_VARIANT_POINTER(AST::TypeNode, next_type);
        union_type->rhs = parse_type();

        return CAST_VARIANT_POINTER(AST::TypeNode, union_type);
    }
    else if (is_type(TokenType::PIPE))
    {
        consume();

        auto variant_type = m_alloc.emplace<AST::VariantTypeNode>();
        variant_type->types.push_back(CAST_VARIANT_POINTER(AST::TypeNode, next_type));

        while (is_type(TokenType::PIPE))
        {
            consume();

            variant_type->types.push_back(parse_type());
        }

        return CAST_VARIANT_POINTER(AST::TypeNode, variant_type);
    }
    else if (is_type(TokenType::OP_LT))
    {
        consume(); // Consume "<"

        while (!is_type(TokenType::OP_GT))
        {
            if (consume().type != TokenType::COMMA)
            {
                next_type->args.push_back(parse_type());
            }
        }
        
        consume(); // Consume ">"
    }

    return CAST_VARIANT_POINTER(AST::TypeNode, next_type);
}

AST::ExprNode* Parser::parse_expr()
{
    return parse_bin_expr(0);
}

AST::ExprNode* Parser::parse_bin_expr(int precedence)
{
    AST::ExprNode* lhs = parse_prim_expr();

    while (true)
    {
        Token op = peek();
        int op_prec = op.bin_prec();

        if (op_prec < precedence)
        {
            break;
        }

        consume();

        AST::ExprNode* rhs = parse_bin_expr(op_prec + 1);

        auto bin_expr_node = m_alloc.emplace<AST::BinExprNode>();
        bin_expr_node->op = op;
        bin_expr_node->lhs = lhs;
        bin_expr_node->rhs = rhs;

        lhs = CAST_VARIANT_POINTER(AST::ExprNode, bin_expr_node);
    }

    return lhs;
}

AST::TypedParamStmtNode* Parser::parse_parameter()
{
    auto parameter_stmt = m_alloc.emplace<AST::TypedParamStmtNode>();
    parameter_stmt->ident = consume();

    if (is_type(TokenType::COLON))
    {
        consume();

        parameter_stmt->type = parse_type();
    }

    return parameter_stmt;
}

AST::LocalDeclStmtNode* Parser::parse_local_declaration()
{
    consume();

    auto decl_stmt = m_alloc.emplace<AST::LocalDeclStmtNode>();
    
    if (is_type(TokenType::KW_CONST))
    {
        consume();
        decl_stmt->is_const = true;
    }

    decl_stmt->ident = consume();

    if (is_type(TokenType::COLON))
    {
        consume();
        decl_stmt->type = parse_type();
    }

    if (is_type(TokenType::OP_ASGN))
    {
        consume();
        decl_stmt->val = parse_expr();
    }

    return decl_stmt;
}

AST::GlobDeclStmtNode* Parser::parse_global_declaration()
{
    auto local_stmt = parse_local_declaration();
    auto global_stmt = m_alloc.emplace<AST::GlobDeclStmtNode>();
    global_stmt->ident = local_stmt->ident;
    global_stmt->type = local_stmt->type;
    global_stmt->val = local_stmt->val;

    delete local_stmt;
    return global_stmt;
}

AST::CallStmtNode* Parser::parse_call_statement()
{
    auto call_stmt = m_alloc.emplace<AST::CallStmtNode>();
    call_stmt->ident = consume();
    
    if (is_type(TokenType::OP_LT))
    {
        call_stmt->type_args = parse_call_type_arguments();
    }

    call_stmt->args = parse_call_arguments();
    return call_stmt;   
}

AST::ReturnStmtNode* Parser::parse_return_statement()
{
    consume();

    auto ret_stmt = m_alloc.emplace<AST::ReturnStmtNode>();
    // TODO: Add multiple return value support
    ret_stmt->vals.push_back(parse_expr());
    return ret_stmt;
}

AST::IndexCallStmtNode* Parser::parse_index_call_statement()
{
    auto index_call_stmt = m_alloc.emplace<AST::IndexCallStmtNode>();
    index_call_stmt->ident = parse_expr();
    
    consume();

    index_call_stmt->index = consume();
    
    if (is_type(TokenType::OP_LT))
    {
        index_call_stmt->type_args = parse_call_type_arguments();
    }

    index_call_stmt->args = parse_call_arguments();
    return index_call_stmt;
}

AST::AssignStmtNode* Parser::parse_assignment_statement()
{
    auto asgn_stmt = m_alloc.emplace<AST::AssignStmtNode>();
    asgn_stmt->ident = consume();

    consume();

    asgn_stmt->val = parse_expr();
    return asgn_stmt;
}

AST::IndexAssignStmtNode* Parser::parse_index_assignment_statement()
{
    auto index_asgn_stmt = m_alloc.emplace<AST::IndexAssignStmtNode>();
    index_asgn_stmt->ident = parse_expr();

    consume();

    index_asgn_stmt->index = consume();
    
    consume();

    index_asgn_stmt->val = parse_expr();
    return index_asgn_stmt;
}

AST::PropertyDeclStmtNode* Parser::parse_property_declaration()
{
    auto local_stmt = parse_local_declaration();
    auto prop_stmt = m_alloc.emplace<AST::PropertyDeclStmtNode>();
    prop_stmt->ident = local_stmt->ident;
    prop_stmt->type = local_stmt->type;
    prop_stmt->val = local_stmt->val;

    delete local_stmt;
    return prop_stmt;
}

AST::WhileStmtNode* Parser::parse_while_statement()
{
    consume();

    auto while_stmt = m_alloc.emplace<AST::WhileStmtNode>();
    while_stmt->cond = parse_expr();
    while_stmt->scope = parse_scope_statement();
    return while_stmt;
}

AST::ForStmtNode* Parser::parse_for_statement()
{
    consume();

    auto for_stmt = m_alloc.emplace<AST::ForStmtNode>();
    // TODO: Make this work ffs
    return for_stmt;
}

AST::IfStmtNode* Parser::parse_if_statement()
{
    consume();

    auto if_stmt = m_alloc.emplace<AST::IfStmtNode>();
    if_stmt->cond = parse_expr();
    if_stmt->body = parse_scope_statement();
    
    while (is_type(TokenType::KW_ELIF))
    {
        consume();

        auto elif_stmt = m_alloc.emplace<AST::ElifStmtNode>();
        elif_stmt->cond = parse_expr();
        elif_stmt->body = parse_scope_statement();

        if_stmt->elif_bodies.push_back(elif_stmt);
    }

    if (is_type(TokenType::KW_ELSE))
    {
        consume();
        
        if_stmt->else_body = parse_scope_statement();
    }

    return if_stmt;
}

AST::SwitchStmtNode* Parser::parse_switch_statement()
{
    consume();

    auto switch_stmt = m_alloc.emplace<AST::SwitchStmtNode>();
    switch_stmt->cond = parse_expr();

    consume();

    while (is_type(TokenType::KW_CASE))
    {
        consume();

        auto case_stmt = m_alloc.emplace<AST::CaseStmtNode>();
        case_stmt->value = parse_expr();
        case_stmt->body = parse_scope_statement();

        switch_stmt->cases.push_back(case_stmt);
    }

    if (is_type(TokenType::KW_DEFAULT))
    {
        consume();

        auto default_stmt = m_alloc.emplace<AST::DefaultStmtNode>();
        default_stmt->body = parse_scope_statement();

        switch_stmt->default_case = default_stmt;
    }

    consume();
    
    return switch_stmt;
}

AST::FuncDeclStmtNode* Parser::parse_function_declaration()
{
    consume();

    auto func_stmt = m_alloc.emplace<AST::FuncDeclStmtNode>();
    
    if (is_type(TokenType::KW_CONST))
    {
        consume();
        func_stmt->is_const = true;
    }

    func_stmt->ident = consume();

    if (is_type(TokenType::OP_LT))
    {
        consume();

        while (!is_type(TokenType::OP_GT))
        {
            if (is_type(TokenType::COMMA))
            {
                consume();
                continue;
            }

            func_stmt->type_params.push_back(consume());
        }
        
        consume();
    }

    consume();

    while (!is_type(TokenType::PAREN_CLOSE))
    {
        if (is_type(TokenType::COMMA))
        {
            consume();
            continue;
        }

        func_stmt->params.push_back(parse_parameter());
    }
    
    consume();

    func_stmt->body = parse_scope_statement();

    return func_stmt;
}

AST::MethodDeclStmtNode* Parser::parse_method_declaration()
{
    auto func_stmt = parse_function_declaration();
    auto method_stmt = m_alloc.emplace<AST::MethodDeclStmtNode>();
    method_stmt->body = func_stmt->body;
    method_stmt->ident = func_stmt->ident;
    method_stmt->is_const = func_stmt->is_const;
    method_stmt->params = func_stmt->params;
    method_stmt->type_params = func_stmt->type_params;

    auto first_param = method_stmt->params.at(0);

    if (first_param->ident.value != "self")
    {
        auto self_param = m_alloc.emplace<AST::TypedParamStmtNode>();
        self_param->ident = Token {
            .type = TokenType::IDENTIFIER,
            .value = "self",
            .line = first_param->ident.line,
            .offset = first_param->ident.offset,
            .has_thrown_error = false
        };

        method_stmt->params.insert(
            method_stmt->params.begin(), self_param);
    }

    delete func_stmt;
    return method_stmt;
}

AST::StructDeclStmtNode* Parser::parse_struct_declaration()
{
    consume();

    auto struct_stmt = m_alloc.emplace<AST::StructDeclStmtNode>();
    struct_stmt->ident = consume();

    if (is_type(TokenType::OP_LT))
    {
        consume();

        while (!is_type(TokenType::OP_GT))
        {
            if (is_type(TokenType::COMMA))
            {
                consume();
                continue;
            }

            struct_stmt->template_types.push_back(consume());
        }

        consume();
    }

    consume();

    while (!is_type(TokenType::BRACE_CLOSE))
    {
        if (is_type(TokenType::KW_PROPERTY))
        {
            auto prop = parse_property_declaration();
            struct_stmt->properties.push_back(prop);
        }
        else if (is_type(TokenType::KW_FUNC))
        {
            auto method = parse_method_declaration();
            struct_stmt->funcs.push_back(method);
        }
    }

    consume();

    return struct_stmt;
}

AST::NamespaceDeclStmtNode* Parser::parse_namespace_declaration()
{
    auto struct_stmt = parse_struct_declaration();
    auto namesp_stmt = m_alloc.emplace<AST::NamespaceDeclStmtNode>();
    namesp_stmt->funcs = struct_stmt->funcs;
    namesp_stmt->ident = struct_stmt->ident;
    namesp_stmt->properties = struct_stmt->properties;
    namesp_stmt->template_types = struct_stmt->template_types;

    for (const auto &prop : namesp_stmt->properties) { prop->is_const = true; }
    for (const auto& func : namesp_stmt->funcs) { func->is_const = true; }

    delete struct_stmt;
    return namesp_stmt;
}

AST::ScopeStmtNode* Parser::parse_scope_statement()
{
    consume();

    auto scope_stmt = m_alloc.emplace<AST::ScopeStmtNode>();

    while (!is_type(TokenType::BRACE_CLOSE))
    {
        scope_stmt->stmts.push_back(parse_statement());
    }

    return scope_stmt;
}

AST::StmtNode* Parser::parse_statement()
{
    switch (peek().type)
    {
    case TokenType::KW_LOCAL:       return CAST_VARIANT_POINTER(AST::StmtNode, parse_local_declaration());
    case TokenType::KW_GLOBAL:      return CAST_VARIANT_POINTER(AST::StmtNode, parse_global_declaration());
    case TokenType::KW_RETURN:      return CAST_VARIANT_POINTER(AST::StmtNode, parse_return_statement());
    case TokenType::KW_PROPERTY:    return CAST_VARIANT_POINTER(AST::StmtNode, parse_property_declaration());
    case TokenType::KW_WHILE:       return CAST_VARIANT_POINTER(AST::StmtNode, parse_while_statement());
    case TokenType::KW_FOR:         return CAST_VARIANT_POINTER(AST::StmtNode, parse_for_statement());
    case TokenType::KW_IF:          return CAST_VARIANT_POINTER(AST::StmtNode, parse_if_statement());
    case TokenType::KW_SWITCH:      return CAST_VARIANT_POINTER(AST::StmtNode, parse_switch_statement());
    case TokenType::KW_FUNC:        return CAST_VARIANT_POINTER(AST::StmtNode, parse_function_declaration());
    case TokenType::KW_NAMESPACE:   return CAST_VARIANT_POINTER(AST::StmtNode, parse_namespace_declaration());
    case TokenType::KW_STRUCT:      return CAST_VARIANT_POINTER(AST::StmtNode, parse_struct_declaration());
    case TokenType::KW_DO:
        consume();
        return CAST_VARIANT_POINTER(AST::StmtNode, parse_scope_statement());
    case TokenType::IDENTIFIER: {
        if (is_type(TokenType::DOT, 1))
        {
            if (is_type(TokenType::OP_LT, 3) || is_type(TokenType::PAREN_OPEN, 3))
            {
                return CAST_VARIANT_POINTER(AST::StmtNode, parse_index_call_statement());
            }
            else if (is_type(TokenType::OP_ASGN, 3))
            {
                return CAST_VARIANT_POINTER(AST::StmtNode, parse_index_assignment_statement());
            }
        }
    }

    default:
        break;
    }

    // Fallback
    return nullptr;
}