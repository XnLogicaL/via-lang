/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see
 * LICENSE for license information */

#pragma once

#include "Parser/ast.h"
#include "arena.hpp"
#include "bytecode.h"
#include "chunk.h"
#include "cleaner.h"
#include "common.h"
#include "instruction.h"
#include "register.h"
#include "stack.h"


// Unlikely Register Id that serves as a non-masked value that carries
// information about register validity
#define VIA_REGISTER_INVALID (0xDEADBEEF)
// Quick way to check if the `target_register` parameter is a valid RegId
// by comparing it with `VIA_REGISTER_INVALID`
#define LOAD_TO_REGISTER (target_register != VIA_REGISTER_INVALID)

#ifndef VIA_GENERATOR_ALLOC_SIZE
    #define VIA_GENERATOR_ALLOC_SIZE 8 * 1024 * 1024 // 8 MB
#endif

namespace via {

class Generator {
public:
    Generator(ProgramData &program)
        : program(program)
        , failed(false)
        , alloc(VIA_GENERATOR_ALLOC_SIZE) // Custom size for the allocator
        , current_chunk(nullptr)
        , initialize_with_chunk(false)
        , saved_stack_pointer(0)
    {
        for (RegId gpr = 0; gpr < VIA_REGISTER_COUNT; gpr++)
            register_pool[gpr] = true;
    }

    ~Generator()
    {
        // Clean up resources using the cleaner
        // Bytecode is managed by the generator object, no need to delete manually
        cleaner.clean();
    }

    // Main function to generate the bytecode
    bool generate();
    // Utility functions
    size_t iota();
    bool is_constexpr(ExprNode);
    ExprNode evaluate_constexpr(ExprNode);
    Operand generate_operand(LiteralExprNode);
    const TValue generate_tvalue(LiteralExprNode);
    // Register management functions
    RegId allocate_temp_register();
    RegId allocate_register();
    size_t load_constant(LiteralExprNode);
    void free_register(RegId);
    void add_bc_info(std::string);

public:
    ProgramData &program;
    kTable constants;
    bool failed;

private:
    ArenaAllocator alloc; // Custom allocator
    Cleaner cleaner;      // Resource cleaner
    std::unordered_map<kGlobId, TValue> globals;
    TestStack stack;
    std::map<RegId, bool> register_pool;
    Chunk *current_chunk;
    bool initialize_with_chunk;
    StkPos saved_stack_pointer;

private:
    // Instruction handling functions
    void push_instruction(OpCode, std::vector<Operand>);
    void load_operand(Operand, Operand);

    // Expression generators
    void generate_literal_expression(LiteralExprNode, RegId);
    void generate_unary_expression(UnaryExprNode, RegId);
    void generate_binary_expression(BinaryExprNode, RegId);
    void generate_lambda_expression(LambdaExprNode, RegId);
    void generate_index_expression(IndexExprNode, RegId);
    void generate_call_expression(CallExprNode, RegId);
    void generate_variable_expression(VarExprNode, RegId);
    void generate_increment_expression(IncExprNode, RegId);
    void generate_decrement_expression(DecExprNode, RegId);
    void generate_expression(ExprNode, RegId);

    // Statement generators
    void generate_variable_declaration_statement(VariableDeclStmtNode);
    void generate_function_declaration_statement(FunctionDeclStmtNode);
    void generate_call_statement(CallStmtNode);
    void generate_assign_statement(AssignStmtNode);
    void generate_while_statement(WhileStmtNode);
    void generate_for_statement(ForStmtNode);
    void generate_scope_statement(ScopeStmtNode, bool);
    void generate_if_statement(IfStmtNode);
    void generate_switch_statement(SwitchStmtNode);
    void generate_return_statement(ReturnStmtNode);
    void generate_break_statement();
    void generate_continue_statement();
    void generate_statement(StmtNode);
};

} // namespace via
