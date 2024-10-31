#include "common.h"
#include "parser.h"

#include "arena.hpp"

using namespace via::Tokenization;
using namespace via::Parsing;

template <typename T>
AST::ExprNode* Parser::cast_expr_ptr(T* v)
{
    return m_alloc.emplace<AST::ExprNode>(*v);
}

template <typename T>
AST::StmtNode* Parser::cast_stmt_ptr(T* v)
{
    return m_alloc.emplace<AST::StmtNode>(*v);
}

template <typename T>
AST::TypeNode* Parser::cast_type_ptr(T* v)
{
    return m_alloc.emplace<AST::TypeNode>(*v);
}

inline Token Parser::consume()
{
    return toks.at(pos++);
}

inline Token Parser::peek(size_t ahead)
{
    if ((pos + ahead) > toks.size())
    {
        throw std::out_of_range(std::format("peek({}): Position out of range (pos={})", ahead, pos));
    }

    return toks.at(pos + ahead);
}

bool Parser::check_value(std::string expected, size_t ahead)
{
    return peek(ahead).value == expected;
}

bool Parser::check_type(TokenType expected, size_t ahead)
{
    return peek(ahead).type == expected;
}

std::vector<AST::ExprNode*> Parser::get_call_args()
{
    consume(); // Consume (

    std::vector<AST::ExprNode*> args;

    while (!check_type(TokenType::PAREN_CLOSE))
    {
        if (check_type(TokenType::COMMA))
        {
            continue;
        }
        else
        {
            args.push_back(parse_expr());
        }
    }

    consume(); // Consume )
    
    return args;
}

std::vector<AST::TypeNode*> Parser::get_call_type_args()
{
    consume(); // Consume <

    std::vector<AST::TypeNode*> types;

    while (!check_type(TokenType::OP_GT))
    {
        if (check_type(TokenType::COMMA))
        {
            consume();
            continue;
        }
        else
        {
            types.push_back(parse_type());
        }
    }

    consume(); // Consume >
    
    return types;
}

AST::TypeNode* Parser::parse_type()
{
    auto current = consume();

    if (current.type == TokenType::BRACE_OPEN) // TableTypeNode
    {
        auto table_type_node = m_alloc.emplace<AST::TableTypeNode>();
        table_type_node->type = parse_type();
        return cast_type_ptr(table_type_node);
    }
    else if (current.type == TokenType::PAREN_OPEN) // FunctorTypeNode
    {
        auto functor_type_node = m_alloc.emplace<AST::FunctorTypeNode>();

        while (!check_type(TokenType::PAREN_CLOSE))
        {
            auto current_ = consume();

            if (current_.type == TokenType::COMMA)
            {
                continue;
            }

            functor_type_node->input.push_back(parse_type());
        }

        consume(); // Consume )
        consume(); // Consume -
        consume(); // Consume >

        auto next = consume();

        if (next.type == TokenType::PAREN_OPEN)
        {
            while (!check_type(TokenType::PAREN_CLOSE))
            {
                auto current_ = consume();

                if (current_.type == TokenType::COMMA)
                {
                    continue;
                }

                functor_type_node->output.push_back(parse_type());
            }

            consume(); // Consume )
        }
        else
        {
            functor_type_node->output.push_back(parse_type());
        }

        return cast_type_ptr(functor_type_node);
    }
    else if (current.type == TokenType::IDENTIFIER) // Other types or generic types
    {
        auto next = consume();
        auto next_type = m_alloc.emplace<AST::LiteralTypeNode>();
        next_type->type = next;
        next_type->args = {};

        // Check for union or variant type before generic type
        if (next.type == TokenType::AMPERSAND) // Union type
        {
            auto union_type_node = m_alloc.emplace<AST::UnionTypeNode>();
            union_type_node->lhs = cast_type_ptr(next_type);
            union_type_node->rhs = parse_type();
            return cast_type_ptr(union_type_node);
        }
        else if (next.type == TokenType::PIPE) // Variant type
        {
            auto variant_type_node = m_alloc.emplace<AST::VariantTypeNode>();
            variant_type_node->types.push_back(cast_type_ptr(next_type));
            variant_type_node->types.push_back(parse_type());

            while (true)
            {
                auto next = peek();

                if (next.type == TokenType::PIPE)
                {
                    consume(); // Consume |
                    continue;
                }

                if (
                    next.type == TokenType::IDENTIFIER || 
                    next.type == TokenType::BRACE_OPEN || 
                    next.type == TokenType::PAREN_OPEN || 
                    next.type == TokenType::OP_LT)
                {
                    variant_type_node->types.push_back(parse_type()); // Parse the next type
                }
                else
                {
                    break; // Break if it's neither a pipe nor a valid type start
                }
            }

            return cast_type_ptr(variant_type_node);
        }

        // Check for generic type declaration
        if (check_type(TokenType::OP_LT)) // Check for <
        {
            consume(); // Consume <
            auto generic_type_node = m_alloc.emplace<AST::LiteralTypeNode>();
            generic_type_node->type = next;

            while (check_type(TokenType::OP_GT))
            {
                if (check_type(TokenType::COMMA))
                {
                    consume(); // Consume ,
                    continue;
                }

                generic_type_node->args.push_back(parse_type());
            }

            consume(); // Consume >

            return cast_type_ptr(generic_type_node);
        }

        // If none of the special cases matched, just return the base type
        return cast_type_ptr(next_type);
    }

    return nullptr;
}

AST::ExprNode* Parser::parse_ident_expr(const Token& current)
{
    AST::ExprNode* expr;

    auto next = peek();

    if (next.type == TokenType::DOT)
    {
        consume(); // Consume "."

        auto ident = consume();
        auto next = peek();

        if (next.type == TokenType::PAREN_OPEN)
        {
            auto index_call_expr_node = m_alloc.emplace<AST::IndexCallExprNode>();
            index_call_expr_node->ident = current;
            index_call_expr_node->index = ident;
            index_call_expr_node->args = get_call_args();
            index_call_expr_node->type_args = {};

            expr = cast_expr_ptr(index_call_expr_node);
        }
        else
        {
            auto index_expr_node = m_alloc.emplace<AST::IndexExprNode>();
            index_expr_node->ident = current;
            index_expr_node->index = ident;

            expr = cast_expr_ptr(index_expr_node);
        }
    }
    else if (next.type == TokenType::BRACKET_OPEN)
    {
        consume(); // Consume "["

        auto index = parse_expr();

        consume(); // Consume "]"

        auto next = peek();

        if (next.type == TokenType::PAREN_OPEN)
        {
            auto index_call_expr_node = m_alloc.emplace<AST::BracketIndexCallExprNode>();
            index_call_expr_node->ident = current;
            index_call_expr_node->index = index;
            index_call_expr_node->args = get_call_args();
            index_call_expr_node->type_args = {};

            expr = cast_expr_ptr(index_call_expr_node);
        }
        else
        {
            auto index_expr_node = m_alloc.emplace<AST::BracketIndexExprNode>();
            index_expr_node->ident = current;
            index_expr_node->index = index;

            expr = cast_expr_ptr(index_expr_node);
        }
    }
    else
    {
        auto lit_expr_node = m_alloc.emplace<AST::LitExprNode>();
        lit_expr_node->val = current;

        expr = cast_expr_ptr(lit_expr_node);
    }

    return expr;
}

AST::ExprNode* Parser::parse_expr()
{
    return parse_bin_expr(0);
}

AST::ExprNode* Parser::parse_bin_expr(int precedence)
{
    AST::ExprNode* lhs = parse_primary_expr();

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

        lhs = cast_expr_ptr(bin_expr_node);
    }

    return lhs;
}

AST::ExprNode* Parser::parse_primary_expr()
{
    auto current = consume();

    if (current.is_literal())
    {
        auto lit_expr = m_alloc.emplace<AST::LitExprNode>();
        lit_expr->val = current;

        return cast_expr_ptr(lit_expr);
    }

    switch (current.type)
    {
    case TokenType::OP_SUB: {
        auto un_expr = m_alloc.emplace<AST::UnExprNode>();
        un_expr->expr = parse_expr();

        return cast_expr_ptr(un_expr);
    }

    case TokenType::PAREN_OPEN: {
        auto group_expr_node = m_alloc.emplace<AST::GroupExprNode>();
        group_expr_node->expr = parse_expr();

        consume();

        return cast_expr_ptr(group_expr_node);
    }

    case TokenType::IDENTIFIER:
        return parse_ident_expr(current);

    default:
        return nullptr;
    }
}

AST::TypedParamStmtNode* Parser::parse_param()
{
    auto param = m_alloc.emplace<AST::TypedParamStmtNode>();
    param->ident = consume(); // Consume the parameter name

    if (check_type(TokenType::COLON, 0))
    {
        consume(); // Consume :

        param->type = parse_type(); // Consume the type
    }

    return param;
}

AST::LocalDeclStmtNode* Parser::parse_local_decl_stmt()
{
    consume();

    auto decl_stmt = m_alloc.emplace<AST::LocalDeclStmtNode>();
    decl_stmt->ident = consume(); // Parse identifier

    if (check_type(TokenType::COLON))
    {
        consume(); // Consume :
        decl_stmt->type = parse_type();
    }

    if (check_type(TokenType::OP_ASGN))
    {
        consume(); // Consume =
        decl_stmt->val = parse_expr(); // Parse expression
    }

    return decl_stmt;
}

AST::GlobDeclStmtNode* Parser::parse_glob_decl_stmt()
{
    consume(); // Consume global

    auto decl_stmt = m_alloc.emplace<AST::GlobDeclStmtNode>();
    decl_stmt->ident = consume();

    if (check_type(TokenType::COLON))
    {
        decl_stmt->type = parse_type();
    }

    consume(); // Consume =

    decl_stmt->val = parse_expr();

    return decl_stmt;
}

AST::CallStmtNode* Parser::parse_call_stmt()
{
    auto call_stmt = m_alloc.emplace<AST::CallStmtNode>();
    call_stmt->ident = consume();
    
    if (check_type(TokenType::OP_LT))
    {
        call_stmt->type_args = get_call_type_args();
    }

    call_stmt->args = get_call_args();

    return call_stmt;
}

AST::ReturnStmtNode* Parser::parse_ret_stmt()
{
    auto return_stmt = m_alloc.emplace<AST::ReturnStmtNode>();

    consume(); // Consume return

    // TODO: Make multi-expr return statements
    return_stmt->vals.push_back(parse_expr());

    return return_stmt;
}

AST::IndexCallStmtNode* Parser::parse_index_call_stmt()
{
    auto ident = consume();
    auto next = consume();

    if (next.type == TokenType::BRACKET_OPEN)
    {
        auto bracket_index_call_stmt_node = m_alloc.emplace<AST::BracketIndexCallStmtNode>();
        bracket_index_call_stmt_node->ident = ident;

        consume(); // Consume [

        bracket_index_call_stmt_node->index = parse_expr();

        consume(); // Consume ]

        if (peek().type == TokenType::OP_LT)
        {
            bracket_index_call_stmt_node->type_args = get_call_type_args();
        }

        bracket_index_call_stmt_node->args = get_call_args();

        return nullptr;
    }
    else if (next.type == TokenType::DOT)
    {
        auto index_call_stmt_node = m_alloc.emplace<AST::IndexCallStmtNode>();
        index_call_stmt_node->ident = ident;
        index_call_stmt_node->index = consume();
        
        if (peek().type == TokenType::OP_LT)
        {
            index_call_stmt_node->type_args = get_call_type_args();
        }

        index_call_stmt_node->args = get_call_args();

        return index_call_stmt_node;
    }

    // Fallback
    return nullptr;
}

AST::AssignStmtNode* Parser::parse_assign_stmt()
{
    auto assign_stmt = m_alloc.emplace<AST::AssignStmtNode>();
    assign_stmt->ident = consume();

    consume(); // Consume =

    assign_stmt->val = parse_expr();

    return assign_stmt;
}

AST::IndexAssignStmtNode* Parser::parse_index_assign_stmt()
{
    auto index_assign_stmt = m_alloc.emplace<AST::IndexAssignStmtNode>();
    index_assign_stmt->ident = consume();

    consume(); // Consume .

    index_assign_stmt->index = consume();

    consume(); // Consume =

    index_assign_stmt->val = parse_expr();

    return index_assign_stmt;
}

AST::PropertyDeclStmtNode* Parser::parse_property_decl_stmt()
{
    consume(); // Consume property

    auto prop_decl_stmt = m_alloc.emplace<AST::PropertyDeclStmtNode>();
    prop_decl_stmt->ident = consume();

    consume(); // Consume :

    prop_decl_stmt->type = parse_type();

    if (check_type(TokenType::OP_ASGN))
    {
        prop_decl_stmt->val = parse_expr();
    }

    return prop_decl_stmt;
}

AST::WhileStmtNode* Parser::parse_while_stmt()
{
    consume(); // Consume while

    auto while_stmt = m_alloc.emplace<AST::WhileStmtNode>();
    while_stmt->cond = parse_expr();
    while_stmt->scope = parse_scope_stmt();

    return while_stmt;
}

AST::ForStmtNode* Parser::parse_for_stmt()
{
    auto for_stmt = m_alloc.emplace<AST::ForStmtNode>();

    // TODO: fuck... make this work

    return for_stmt;
}

AST::IfStmtNode* Parser::parse_if_stmt()
{
    consume(); // Consume if

    auto if_stmt = m_alloc.emplace<AST::IfStmtNode>();
    if_stmt->cond = parse_expr();
    if_stmt->body = parse_scope_stmt();

    while (check_type(TokenType::KW_ELIF))
    {
        consume(); // Consume elseif

        auto elif_stmt = m_alloc.emplace<AST::ElifStmtNode>();
        elif_stmt->cond = parse_expr();
        elif_stmt->body = parse_scope_stmt();

        if_stmt->elif_bodies.push_back(elif_stmt);
    }
    
    if (check_type(TokenType::KW_ELSE))
    {
        if_stmt->else_body = parse_scope_stmt();
    }

    return if_stmt;
}

AST::SwitchStmtNode* Parser::parse_switch_stmt()
{
    consume(); // Consume switch

    auto switch_stmt = m_alloc.emplace<AST::SwitchStmtNode>();
    switch_stmt->cond = parse_expr();

    consume(); // Consume {
    
    while (check_type(TokenType::KW_CASE))
    {
        consume(); // Consume case

        auto case_stmt = m_alloc.emplace<AST::CaseStmtNode>();
        case_stmt->value = parse_expr();
        case_stmt->body = parse_scope_stmt();

        switch_stmt->cases.push_back(case_stmt);
    }

    if (check_type(TokenType::KW_DEFAULT))
    {
        consume(); // Consume default

        auto default_stmt = m_alloc.emplace<AST::DefaultStmtNode>();
        default_stmt->body = parse_scope_stmt(); 

        switch_stmt->default_case = default_stmt;
    }

    consume(); // Consume }

    return switch_stmt;
}

AST::FuncDeclStmtNode* Parser::parse_func_decl_stmt()
{
    consume(); // Consume func

    auto is_const = check_type(TokenType::KW_CONST);

    if (is_const)
    {
        consume(); // Consume const
    }

    auto func_decl_stmt = m_alloc.emplace<AST::FuncDeclStmtNode>();
    func_decl_stmt->ident = consume();

    if (check_type(TokenType::OP_LT))
    {
        while (!check_type(TokenType::OP_GT))
        {
            if (check_type(TokenType::COMMA))
            {
                consume();
                continue;
            }

            func_decl_stmt->type_params.push_back(consume());
        }
    }

    consume(); // Consume (

    while (!check_type(TokenType::PAREN_CLOSE))
    {
        if (check_type(TokenType::COMMA))
        {
            consume();
            continue;
        }

        auto typed_param_stmt = m_alloc.emplace<AST::TypedParamStmtNode>();
        typed_param_stmt->ident = consume();

        if (check_type(TokenType::COLON))
        {
            consume(); // Consume :

            typed_param_stmt->type = parse_type();
        }
    }

    consume(); // Consume )

    func_decl_stmt->body = parse_scope_stmt();

    return func_decl_stmt;
}

AST::MethodDeclStmtNode* Parser::parse_method_decl_stmt()
{
    auto func_decl_stmt = parse_func_decl_stmt();
    auto method_decl_stmt = m_alloc.emplace<AST::MethodDeclStmtNode>();
    method_decl_stmt->ident = func_decl_stmt->ident;
    method_decl_stmt->body = func_decl_stmt->body;
    method_decl_stmt->type_params = func_decl_stmt->type_params;

    for (const auto &param : func_decl_stmt->params)
    {
        method_decl_stmt->params.push_back(param);
    }

    delete func_decl_stmt;

    return method_decl_stmt;
}

AST::NamespaceDeclStmtNode* Parser::parse_namesp_decl_stmt()
{
    consume(); // Consume namespace

    auto namespace_decl_stmt = m_alloc.emplace<AST::NamespaceDeclStmtNode>();
    namespace_decl_stmt->ident = consume();

    if (check_type(TokenType::OP_LT))
    {
        consume(); // Consume <

        while (!check_type(TokenType::OP_GT))
        {
            namespace_decl_stmt->template_types.push_back(parse_type());
        }

        consume(); // Consume >
    }

    consume(); // Consume {

    while (!check_type(TokenType::BRACKET_CLOSE))
    {
        if (check_type(TokenType::KW_FUNC))
        {
            namespace_decl_stmt->funcs.push_back(parse_method_decl_stmt());
        }
        else if (check_type(TokenType::KW_PROPERTY))
        {
            namespace_decl_stmt->properties.push_back(parse_property_decl_stmt());
        }
    }

    consume(); // Consume }

    return namespace_decl_stmt;
}

AST::StructDeclStmtNode* Parser::parse_struct_decl_stmt()
{
    auto namespace_decl_stmt = parse_namesp_decl_stmt();
    auto struct_decl_stmt = m_alloc.emplace<AST::StructDeclStmtNode>();
    struct_decl_stmt->funcs = namespace_decl_stmt->funcs;
    struct_decl_stmt->ident = namespace_decl_stmt->ident;
    struct_decl_stmt->properties = namespace_decl_stmt->properties;
    struct_decl_stmt->template_types = namespace_decl_stmt->template_types;

    delete namespace_decl_stmt;

    return struct_decl_stmt;
}

AST::ScopeStmtNode* Parser::parse_scope_stmt()
{
    consume(); // Consume {

    auto scope_stmt_node = m_alloc.emplace<AST::ScopeStmtNode>();

    while (!check_type(TokenType::BRACE_CLOSE))
    {
        scope_stmt_node->stmts.push_back(parse_stmt());        
    }
    
    consume(); // Consume }

    return scope_stmt_node;
}

AST::StmtNode* Parser::parse_stmt()
{
    switch (peek().type)
    {
    case TokenType::KW_LOCAL:
        return cast_stmt_ptr(parse_local_decl_stmt());
    case TokenType::KW_GLOBAL:
        return cast_stmt_ptr(parse_glob_decl_stmt());
    case TokenType::IDENTIFIER: {
        if (check_type(TokenType::PAREN_OPEN, 1))
        {
            return cast_stmt_ptr(parse_call_stmt());
        }
        else if (check_type(TokenType::OP_ASGN, 1))
        {
            return cast_stmt_ptr(parse_assign_stmt());
        }
        else if (check_type(TokenType::DOT, 1))
        {
            if (check_type(TokenType::PAREN_OPEN, 3))
            {
                return cast_stmt_ptr(parse_index_call_stmt());
            }
            else if (check_type(TokenType::OP_ASGN))
            {
                return cast_stmt_ptr(parse_index_assign_stmt());
            }
        }
        else if (check_type(TokenType::BRACKET_OPEN, 1))
        {
            size_t next_bracket = 1;

            while (!check_type(TokenType::BRACKET_CLOSE, next_bracket))
            {
                next_bracket++;
            }
            
            if (check_type(TokenType::OP_ASGN, next_bracket))
            {
                return cast_stmt_ptr(parse_index_assign_stmt());
            }
            else if (check_type(TokenType::PAREN_OPEN))
            {
                return cast_stmt_ptr(parse_index_call_stmt());
            }
        }

        return nullptr;
    }
    case TokenType::KW_RETURN:
        return cast_stmt_ptr(parse_ret_stmt());
    case TokenType::KW_PROPERTY:
        return cast_stmt_ptr(parse_property_decl_stmt());
    case TokenType::KW_WHILE:
        return cast_stmt_ptr(parse_while_stmt());
    case TokenType::KW_FOR:
        return cast_stmt_ptr(parse_for_stmt());
    case TokenType::KW_IF:
        return cast_stmt_ptr(parse_if_stmt());
    case TokenType::KW_SWITCH:
        return cast_stmt_ptr(parse_switch_stmt());
    case TokenType::KW_FUNC:
        return cast_stmt_ptr(parse_func_decl_stmt());
    case TokenType::KW_NAMESPACE:
        return cast_stmt_ptr(parse_namesp_decl_stmt());
    case TokenType::KW_STRUCT:
        return cast_stmt_ptr(parse_struct_decl_stmt());
    case TokenType::KW_DO:
        consume();
        return cast_stmt_ptr(parse_scope_stmt());
    default:
        break;
    }

    // Fallback
    return nullptr;
}

AST::AST* Parser::parse_prog()
{
    auto ast = m_alloc.emplace<AST::AST>();

    while (!check_type(TokenType::EOF_))
    {
        ast->stmts.push_back(parse_stmt());
    }

    return ast;
}