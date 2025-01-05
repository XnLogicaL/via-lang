/* This file is a part of the via programming language at
 * https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"

namespace via::Compilation
{

using namespace Parsing;
using namespace AST;
using namespace Tokenization;

void Generator::generate_literal_expression(LiteralExprNode lit_expr = {}, GPRegister target_register = VIA_GPREGISTER_INVALID)
{
    Operand operand = generate_operand(this, lit_expr);
    if (LOAD_TO_REGISTER)
        // If a target register is specified, use the LOAD instruction to load the
        // value into the register
        load_operand(cnewoperand(target_register), operand);
    else
        // If no target register is specified, push the value onto the stack
        push_instruction(OpCode::PUSH, {operand});
}

void Generator::generate_unary_expression(UnaryExprNode unary_expr = {}, GPRegister target_register = VIA_GPREGISTER_INVALID)
{
    if (LOAD_TO_REGISTER)
    { // If a target register is specified, load the unary
      // expression into the register and perform inline
      // negation
        generate_expression(*unary_expr.expr, target_register);
        push_instruction(OpCode::NEGI, {cnewoperand(target_register)});
    }
    else
    { // If no target register is specified, allocate a register
        // and load the expression into that register, inline negate it and push it
        // onto the stack
        GPRegister reg = allocate_register();
        generate_expression(*unary_expr.expr, reg);
        push_instruction(OpCode::NEGI, {cnewoperand(target_register)});
        push_instruction(OpCode::PUSH, {cnewoperand(target_register)});
        free_register(reg);
    }
}

void Generator::generate_binary_expression(BinaryExprNode bin_expr = {}, GPRegister target_register = VIA_GPREGISTER_INVALID)
{
    static const std::unordered_map<TokenType, OpCode> simple_operator_map = {
        {TokenType::OP_LT, OpCode::LT},
        {TokenType::OP_GT, OpCode::GT},
        {TokenType::OP_EQ, OpCode::EQ},
        {TokenType::OP_NEQ, OpCode::NEQ},
        {TokenType::OP_LEQ, OpCode::LE},
        {TokenType::OP_GEQ, OpCode::GE},
    };

    static const std::unordered_map<TokenType, std::array<OpCode, 6>> complex_operator_map = {
        {TokenType::OP_ADD,
         {
             OpCode::ADDRR,
             OpCode::ADDRN,
             OpCode::ADDNR,
             OpCode::ADDNN,
             OpCode::ADDIR,
             OpCode::ADDIN,
         }},
        {TokenType::OP_SUB,
         {
             OpCode::SUBRR,
             OpCode::SUBRN,
             OpCode::SUBNR,
             OpCode::SUBNN,
             OpCode::SUBIR,
             OpCode::SUBIN,
         }},
        {TokenType::OP_MUL,
         {
             OpCode::MULRR,
             OpCode::MULRN,
             OpCode::MULNR,
             OpCode::MULNN,
             OpCode::MULIR,
             OpCode::MULIN,
         }},
        {TokenType::OP_DIV,
         {
             OpCode::DIVRR,
             OpCode::DIVRN,
             OpCode::DIVNR,
             OpCode::DIVNN,
             OpCode::DIVIR,
             OpCode::DIVIN,
         }},
        {TokenType::OP_EXP,
         {
             OpCode::POWRR,
             OpCode::POWRN,
             OpCode::POWNR,
             OpCode::POWNN,
             OpCode::POWIR,
             OpCode::POWIN,
         }},
        {TokenType::OP_MOD,
         {
             OpCode::MODRR,
             OpCode::MODRN,
             OpCode::MODNR,
             OpCode::MODNN,
             OpCode::MODIR,
             OpCode::MODIN,
         }},
    };

    OpCode op;
    Operand dst = cnewoperand(target_register);
    Operand lhs, rhs;
    uint8_t complex_id;
    GPRegister lhs_register = allocate_register();
    GPRegister rhs_register = allocate_register();

    auto it = simple_operator_map.find(bin_expr.op.type);
    auto it_complex = complex_operator_map.find(bin_expr.op.type);

    if (it == simple_operator_map.end())
    {
        LiteralExprNode *lhs_literal = std::get_if<LiteralExprNode>(bin_expr.lhs);
        LiteralExprNode *rhs_literal = std::get_if<LiteralExprNode>(bin_expr.rhs);

        if (lhs_literal && rhs_literal)
        { // <OP>NN
            complex_id = 3;
            // Generate literal operands
            lhs = generate_operand(this, *lhs_literal);
            rhs = generate_operand(this, *rhs_literal);
        }
        else if (rhs_literal)
        { // <OP>RN
            complex_id = 1;
            // Generate lhs
            generate_expression(*bin_expr.lhs, lhs_register);
            // Generate rhs
            rhs = generate_operand(this, *rhs_literal);
            lhs = cnewoperand(lhs_register);
        }
        else if (lhs_literal)
        { // <OP>NR
            complex_id = 2;
            // Generate lhs
            lhs = generate_operand(this, *lhs_literal);
            rhs = cnewoperand(rhs_register);
            // Generate rhs
            generate_expression(*bin_expr.rhs, rhs_register);
        }
        else
        { // <OP>RR
            complex_id = 0;
            // Generate lhs and rhs
            generate_expression(*bin_expr.lhs, lhs_register);
            generate_expression(*bin_expr.rhs, rhs_register);
            // Initialize operands
            lhs = cnewoperand(lhs_register);
            rhs = cnewoperand(rhs_register);
        }

        op = it_complex->second.at(complex_id);
    }
    else
        op = it->second;

    if (LOAD_TO_REGISTER)
    {
        if (complex_id > 3)
        {
            // Check if OpCode is an inline arithmetic opcode
            load_operand(dst, lhs);
            push_instruction(op, {dst, rhs});
            return;
        }

        push_instruction(op, {dst, lhs, rhs});
        free_register(lhs_register);
        free_register(rhs_register);
    }
    else
    {
        GPRegister temp_register = allocate_temp_register();
        Operand temp = cnewoperand(temp_register);

        push_instruction(op, {temp, lhs, rhs});
        push_instruction(OpCode::PUSH, {temp});
    }
}

void Generator::generate_lambda_expression(LambdaExprNode lmd_expr = {}, GPRegister target_register = VIA_GPREGISTER_INVALID)
{
    GPRegister destination = target_register;

    if (!LOAD_TO_REGISTER)
        destination = allocate_register();

    // Push function load instruction
    push_instruction(OpCode::LOADFUNCTION, {cnewoperand(destination)});

    // Generate statements
    for (StmtNode lambda_stmt : lmd_expr.body->statements)
        generate_statement(lambda_stmt);

    // Check if the lambda body has been terminated by a RET instruction
    // If not, insert one
    if (bytecode->instructions.back().op != OpCode::RET)
        push_instruction(OpCode::RET, {});

    // Check if the function is supposed to be pushed onto the stack
    if (!LOAD_TO_REGISTER)
    { // Push the function onto the stack and free the
      // temporary holder register
        push_instruction(OpCode::PUSH, {cnewoperand(destination)});
        free_register(destination);
    }
}

void Generator::generate_index_expression(IndexExprNode idx_expr = {}, GPRegister target_register = VIA_GPREGISTER_INVALID) {}

void Generator::generate_call_expression(CallExprNode call_expr, GPRegister target_register = VIA_GPREGISTER_INVALID) {}

void Generator::generate_variable_expression(VarExprNode var_expr, GPRegister target_register = VIA_GPREGISTER_INVALID) {}

void Generator::generate_expression(ExprNode expr, GPRegister target_register = VIA_GPREGISTER_INVALID)
{
    if (LiteralExprNode *lit_expr = std::get_if<LiteralExprNode>(&expr))
        generate_literal_expression(*lit_expr, target_register);
    else if (UnaryExprNode *un_expr = std::get_if<UnaryExprNode>(&expr))
        generate_unary_expression(*un_expr, target_register);
    else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(&expr))
        generate_binary_expression(*bin_expr, target_register);
    else if (LambdaExprNode *lmd_expr = std::get_if<LambdaExprNode>(&expr))
        generate_lambda_expression(*lmd_expr, target_register);
    else if (IndexExprNode *idx_expr = std::get_if<IndexExprNode>(&expr))
        generate_index_expression(*idx_expr, target_register);
    else if (CallExprNode *call_expr = std::get_if<CallExprNode>(&expr))
        generate_call_expression(*call_expr, target_register);
    else if (VarExprNode *var_expr = std::get_if<VarExprNode>(&expr))
        generate_variable_expression(*var_expr, target_register);

    VIA_ASSERT(false, "Unsupported expression type in generate_expression.");
}

} // namespace via::Compilation
