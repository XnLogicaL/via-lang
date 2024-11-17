/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "ast.h"
#include "arena.hpp"
#include "token.h"
#include "container.h"

#ifndef __VIA_PARSER_ALLOC_SIZE
    // Default allocation size set to 8MiB
    #define __VIA_PARSER_ALLOC_SIZE (8 * 1024 * 1024)
#endif

namespace via::Parsing
{

class Parser
{
    // Memory allocator
    ArenaAllocator m_alloc;

    // Token management
    viaSourceContainer container;
    size_t current_position;

    // Core token manipulation functions
    Tokenization::Token consume();
    Tokenization::Token peek(int offset = 0) const;

    // Token checking utilities
    bool is_value(const std::string& value = "", int offset = 0) const;
    bool is_type(Tokenization::TokenType type = Tokenization::TokenType::UNKNOWN, int offset = 0) const;

    // Parsing helper functions
    std::vector<AST::ExprNode*> parse_call_arguments();
    std::vector<AST::TypeNode*> parse_call_type_arguments();
    template <typename IndexExprType, typename IndexCallExprType>
    AST::ExprNode* parse_index_expr(AST::ExprNode* base_expr);
    AST::ExprNode* parse_literal_or_group_expr(Token current);

    // Type and expression parsing functions
    AST::TypeNode* parse_type();
    AST::ExprNode* parse_expr();
    AST::ExprNode* parse_bin_expr(int precedence = 0);
    AST::ExprNode* parse_prim_expr();

    // Statement parsing functions
    AST::TypedParamStmtNode* parse_parameter();
    AST::LocalDeclStmtNode* parse_local_declaration();
    AST::GlobDeclStmtNode* parse_global_declaration();
    AST::CallStmtNode* parse_call_statement();
    AST::ReturnStmtNode* parse_return_statement();
    AST::IndexCallStmtNode* parse_index_call_statement();
    AST::AssignStmtNode* parse_assignment_statement();
    AST::IndexAssignStmtNode* parse_index_assignment_statement();
    AST::PropertyDeclStmtNode* parse_property_declaration();
    AST::WhileStmtNode* parse_while_statement();
    AST::ForStmtNode* parse_for_statement();
    AST::IfStmtNode* parse_if_statement();
    AST::SwitchStmtNode* parse_switch_statement();
    AST::FuncDeclStmtNode* parse_function_declaration();
    AST::MethodDeclStmtNode* parse_method_declaration();
    AST::NamespaceDeclStmtNode* parse_namespace_declaration();
    AST::StructDeclStmtNode* parse_struct_declaration();
    AST::ScopeStmtNode* parse_scope_statement();
    AST::StmtNode* parse_statement();

public:

    Parser(viaSourceContainer vsc)
        : m_alloc(ArenaAllocator(__VIA_PARSER_ALLOC_SIZE))
        , container(vsc)
        , current_position(0) {}

    AST::AST* parse_program();
};

} // namespace via::Parsing
