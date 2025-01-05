#pragma once

#include "Parser/ast.h"
#include "arena.hpp"
#include "bytecode.h"
#include "chunk.h"
#include "cleaner.h"
#include "common.h"
#include "core.h"
#include "instruction.h"
#include "register.h"
#include "stack.h"


// Unlikely GPRegister Id that serves as a non-masked value that carries
// information about register validity
#define VIA_GPREGISTER_INVALID (0xDEADBEEF)
// Quick way to check if the `target_register` parameter is a valid GPRegister
// by comparing it with `VIA_GPREGISTER_INVALID`
#define LOAD_TO_REGISTER (target_register != VIA_GPREGISTER_INVALID)

#ifndef VIA_GENERATOR_ALLOC_SIZE
    #define VIA_GENERATOR_ALLOC_SIZE 8 * 1024 * 1024 // 8 MB
#endif

namespace via::Compilation
{

class Generator
{
public:
    Generator(Parsing::AST::AST *tree)
        : alloc(VIA_GENERATOR_ALLOC_SIZE) // Custom size for the allocator
        , __iota__(0)
        , bytecode(std::make_unique<Bytecode>())
    {
        bytecode->ast = tree;
        bytecode->instructions = {};

        register_pool.reserve(VIA_REGISTER_COUNT);
        for (GPRegister gpr = 0; gpr < VIA_REGISTER_COUNT; gpr++)
            register_pool[gpr] = true;
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

    // Register management functions
    GPRegister allocate_temp_register();
    GPRegister allocate_register();
    void free_register(GPRegister);

public:
    ArenaAllocator alloc; // Custom allocator
    Cleaner cleaner;      // Resource cleaner
    Chunk *current_chunk;
    bool initialize_with_chunk;

private:
    size_t __iota__;                    // Counter for unique operations
    std::unique_ptr<Bytecode> bytecode; // Bytecode object
    std::unordered_map<GPRegister, bool> register_pool;

private:
    // Instruction handling functions
    void push_instruction(OpCode, std::vector<Operand>);
    void load_operand(Operand, Operand);

    // Expression generators
    void generate_literal_expression(Parsing::AST::LiteralExprNode, GPRegister);
    void generate_unary_expression(Parsing::AST::UnaryExprNode, GPRegister);
    void generate_binary_expression(Parsing::AST::BinaryExprNode, GPRegister);
    void generate_lambda_expression(Parsing::AST::LambdaExprNode, GPRegister);
    void generate_index_expression(Parsing::AST::IndexExprNode, GPRegister);
    void generate_call_expression(Parsing::AST::CallExprNode, GPRegister);
    void generate_variable_expression(Parsing::AST::VarExprNode, GPRegister);
    void generate_expression(Parsing::AST::ExprNode, GPRegister);

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
