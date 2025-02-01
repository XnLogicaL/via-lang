/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "ast.h"
#include "arena.hpp"
#include "highlighter.h"
#include "token.h"

#ifndef VIA_PARSER_ALLOC_SIZE
    #define VIA_PARSER_ALLOC_SIZE (8 * 1024 * 1024) // 8MB
#endif

namespace via
{

class Parser
{
public:
    Parser(ProgramData &program)
        : program(program)
        , emitter(program)
        , current_position(0)
        , failed(false)
    {
    }

    void parse_program();

private:
    ProgramData &program;
    ArenaAllocator *alloc;   // Memory allocator
    Emitter emitter;         // Error emitter
    Pragma pragma;           // Current pragma
    size_t current_position; // Token management
    bool has_pragma;
    bool failed;

private:
    // Core token manipulation functions
    Token consume();
    Token peek(int offset = 0) const;

    // Token checking utilities
    bool is_value(const std::string &value = "", int offset = 0) const;
    bool is_type(TokenType type = TokenType::UNKNOWN, int offset = 0) const;

    // Panic related
    bool is_keyword();
    void panic_and_recover();

    // Parsing helper functions
    std::vector<ExprNode *> parse_call_arguments();
    std::vector<TypeNode *> parse_call_type_arguments();
    Pragma parse_pragma();
    StatementModifiers parse_modifiers(std::function<bool(void)>);
    DeclarationType get_decl_type(TokenType);

    template<typename IndexExprType, typename IndexCallExprType>
    ExprNode *parse_index_expr(ExprNode *);
    ExprNode *parse_literal_or_group_expr(Token);
    ExprNode *parse_table_expr();

    // Type and expression parsing functions
    TypeNode *parse_type_generic();
    TypeNode *parse_type();
    ExprNode *parse_expr();
    ExprNode *parse_bin_expr(int precedence = 0);
    ExprNode *parse_prim_expr();

    // Statement parsing functions
    TypedParamNode *parse_parameter();
    VariableDeclStmtNode *parse_var_declaration();
    CallStmtNode *parse_call_statement(ExprNode *);
    AssignStmtNode *parse_assignment_statement(ExprNode *);
    ReturnStmtNode *parse_return_statement();
    WhileStmtNode *parse_while_statement();
    ForStmtNode *parse_for_statement();
    IfStmtNode *parse_if_statement();
    SwitchStmtNode *parse_switch_statement();
    FunctionDeclStmtNode *parse_function_declaration(DeclarationType);
    StructDeclStmtNode *parse_struct_declaration(DeclarationType);
    ScopeStmtNode *parse_scope_statement();
    StmtNode *parse_statement();
};

} // namespace via
