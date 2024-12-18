#pragma once

#include "cleaner.h"
#include "common.h"
#include "core.h"
#include "instruction.h"
#include "bytecode.h"
#include "register.h"
#include "stack.h"
#include "arena.hpp"
#include "Parser/ast.h"

#ifndef VIA_GENERATOR_CUSTOM_ALLOC_SIZE
#    define VIA_GENERATOR_CUSTOM_ALLOC_SIZE 8 * 1024 * 1024 // 8 MB
#endif

namespace via::Compilation
{

class Generator
{
public:
    Generator(
        Parsing::AST::AST *tree,
        RegisterPool &pool,
        RegisterManager &manager,
        RegisterAllocator &allocator,
        TestStack &stack
    )
        : alloc(VIA_GENERATOR_CUSTOM_ALLOC_SIZE) // Custom size for the allocator
        , stack(stack)
        , __iota__(0)
        , register_pool(pool)
        , register_manager(manager)
        , register_allocator(allocator)
        , bytecode(std::make_unique<Bytecode>())
    {
        bytecode->ast = tree;
        bytecode->instructions = {};
    }

    ~Generator()
    {
        // Clean up resources using the cleaner
        cleaner.clean();
        // Bytecode is managed by the generator object, no need to delete manually
    }

    // Main function to generate the bytecode
    std::unique_ptr<Bytecode> generate();

    // Utility functions
    size_t iota();
    bool is_constexpr(Parsing::AST::ExprNode, int);
    void evaluate_constexpr(Parsing::AST::ExprNode *expr);

public:
    ArenaAllocator alloc; // Custom allocator
    TestStack &stack;     // Reference to the stack

private:
    size_t __iota__; // Counter for unique operations
    Cleaner cleaner; // Resource cleaner
    RegisterPool &register_pool;
    RegisterManager &register_manager;
    RegisterAllocator &register_allocator;
    std::unique_ptr<Bytecode> bytecode; // Bytecode object

private:
    // Instruction handling functions
    void push_instruction(OpCode, std::vector<viaOperand>);

    // Expression generators
    viaRegister generate_literal_expression(Parsing::AST::LiteralExprNode);
    viaRegister generate_unary_expression(Parsing::AST::UnaryExprNode);
    viaRegister generate_binary_expression(Parsing::AST::BinaryExprNode);
    viaRegister generate_lambda_expression(Parsing::AST::LambdaExprNode);
    viaRegister generate_index_expression(Parsing::AST::IndexExprNode);
    viaRegister generate_call_expression(Parsing::AST::CallExprNode);
    viaRegister generate_variable_expression(Parsing::AST::VarExprNode);
    viaRegister generate_expression(Parsing::AST::ExprNode);

    // Statement generators
    void generate_local_declaration_statement(Parsing::AST::LocalDeclStmtNode);
    void generate_global_declaration_statement(Parsing::AST::GlobalDeclStmtNode);
    void generate_function_declaration_statement(Parsing::AST::FunctionDeclStmtNode);
    void generate_call_statement(Parsing::AST::CallStmtNode);
    void generate_assign_statement(Parsing::AST::AssignStmtNode);
    void generate_while_statement(Parsing::AST::WhileStmtNode);
    void generate_for_statement(Parsing::AST::ForStmtNode);
    void generate_scope_statement(Parsing::AST::ScopeStmtNode);
    void generate_if_statement(Parsing::AST::IfStmtNode);
    void generate_switch_statement(Parsing::AST::SwitchStmtNode);
    void generate_return_statement(Parsing::AST::ReturnStmtNode);
    void generate_break_statement();
    void generate_continue_statement();
    void generate_statement(Parsing::AST::StmtNode);
};

} // namespace via::Compilation
