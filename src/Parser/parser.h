/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "ast.h"
#include "arena.hpp"
#include "token.h"
#include "container.h"

#ifndef VIA_PARSER_ALLOC_SIZE
#    define VIA_PARSER_ALLOC_SIZE (8 * 1024 * 1024) // 8MB
#endif

namespace via::Parsing
{

class Parser
{
    using Token = Tokenization::Token;
    using TokenType = Tokenization::TokenType;

public:
    Parser(viaSourceContainer &vsc)
        : alloc(VIA_PARSER_ALLOC_SIZE)
        , container(vsc)
        , current_position(0)
    {
    }

    AST::AST *parse_program();

private:
    // Memory allocator
    ArenaAllocator alloc;
    // Token management
    viaSourceContainer &container;
    size_t current_position;

private:
    // Core token manipulation functions
    Token consume();
    Token peek(int offset = 0) const;

    // Token checking utilities
    bool is_value(const std::string &value = "", int offset = 0) const;
    bool is_type(TokenType type = TokenType::UNKNOWN, int offset = 0) const;

    // Parsing helper functions
    std::vector<AST::ExprNode> parse_call_arguments();
    std::vector<AST::TypeNode> parse_call_type_arguments();

    template<typename IndexExprType, typename IndexCallExprType>
    AST::ExprNode *parse_index_expr(AST::ExprNode *);
    AST::ExprNode *parse_literal_or_group_expr(Token);

    // Type and expression parsing functions
    AST::TypeNode *parse_type_generic();
    AST::TypeNode *parse_type();
    AST::ExprNode *parse_expr();
    AST::ExprNode *parse_bin_expr(int precedence = 0);
    AST::ExprNode *parse_prim_expr();

    // Statement parsing functions
    AST::TypedParamNode *parse_parameter();
    AST::LocalDeclStmtNode *parse_local_declaration();
    AST::GlobalDeclStmtNode *parse_global_declaration();
    AST::CallStmtNode *parse_call_statement(AST::ExprNode *);
    AST::AssignStmtNode *parse_assignment_statement(AST::ExprNode *);
    AST::ReturnStmtNode *parse_return_statement();
    AST::WhileStmtNode *parse_while_statement();
    AST::ForStmtNode *parse_for_statement();
    AST::IfStmtNode *parse_if_statement();
    AST::SwitchStmtNode *parse_switch_statement();
    AST::FunctionDeclStmtNode *parse_function_declaration();
    AST::ScopeStmtNode *parse_scope_statement();
    AST::StmtNode *parse_statement();
};

} // namespace via::Parsing
