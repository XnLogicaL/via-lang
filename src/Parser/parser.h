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
    // This is a const reference because the size of the object could be quite large,
    // Especially on large projects with multiple module files
    const Tokenization::viaSourceContainer &container;
    size_t current_position;

    // Core token manipulation functions
    Tokenization::Token consume();
    Tokenization::Token peek(int offset = 0) const;

    // Token checking utilities
    bool is_value(const std::string &value = "", int offset = 0) const;
    bool is_type(Tokenization::TokenType type = Tokenization::TokenType::UNKNOWN, int offset = 0) const;

    // Parsing helper functions

    // Parses arguments after the current identifier if applicable
    std::vector<AST::ExprNode *> parse_call_arguments();
    // Parses the call type arguments after the current identifier if applicable
    // Should be called before `parse_call_arguments`
    std::vector<AST::TypeNode *> parse_call_type_arguments();
    // Parses an index expression
    // Can be anything like `ident.index`, `ident.call()`, `ident.assign = ...`, etc.
    template<typename IndexExprType, typename IndexCallExprType>
    AST::ExprNode *parse_index_expr(AST::ExprNode *base_expr);
    // Parses single expressions (terms)
    AST::ExprNode *parse_literal_or_group_expr(Tokenization::Token current);

    // Type and expression parsing functions

    // Parses a type expression
    AST::TypeNode *parse_type();
    // Parses an expression
    AST::ExprNode *parse_expr();
    // Parses a binary expression
    AST::ExprNode *parse_bin_expr(int precedence = 0);
    // Parses a primary expression
    AST::ExprNode *parse_prim_expr();

    // Statement parsing functions
    // No need to document these, the names are self explanatory
    AST::TypedParamStmtNode *parse_parameter();
    AST::LocalDeclStmtNode *parse_local_declaration();
    AST::GlobDeclStmtNode *parse_global_declaration();
    AST::CallStmtNode *parse_call_statement();
    AST::ReturnStmtNode *parse_return_statement();
    AST::IndexCallStmtNode *parse_index_call_statement();
    AST::AssignStmtNode *parse_assignment_statement();
    AST::IndexAssignStmtNode *parse_index_assignment_statement();
    AST::PropertyDeclStmtNode *parse_property_declaration();
    AST::WhileStmtNode *parse_while_statement();
    AST::ForStmtNode *parse_for_statement();
    AST::IfStmtNode *parse_if_statement();
    AST::SwitchStmtNode *parse_switch_statement();
    AST::FuncDeclStmtNode *parse_function_declaration();
    AST::MethodDeclStmtNode *parse_method_declaration();
    AST::NamespaceDeclStmtNode *parse_namespace_declaration();
    AST::StructDeclStmtNode *parse_struct_declaration();
    AST::ScopeStmtNode *parse_scope_statement();
    AST::StmtNode *parse_statement();

public:
    Parser(const Tokenization::viaSourceContainer &vsc)
        : m_alloc(ArenaAllocator(__VIA_PARSER_ALLOC_SIZE))
        , container(vsc)
        , current_position(0)
    {
    }

    AST::AST *parse_program();
};

} // namespace via::Parsing
