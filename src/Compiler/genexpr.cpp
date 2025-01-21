/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "builtins.h"
#include "gen.h"

namespace via
{

// Generates and emits a literal expression
void Generator::generate_literal_expression(LiteralExprNode lit_expr, RegId target_register)
{
    Operand operand = generate_operand(lit_expr);
    if (LOAD_TO_REGISTER)
    {
        size_t const_idx = load_constant(lit_expr);
        TNumber const_idx_num = const_idx;
        push_instruction(OpCode::LOADK, {Operand(target_register), Operand(const_idx_num)});
    }
    else
        // If no target register is specified, push the value onto the stack
        push_instruction(OpCode::PUSHSTACK, {Operand(operand)});
}

// Generates and emits a unary expression
void Generator::generate_unary_expression(UnaryExprNode unary_expr, RegId target_register)
{
    if (LOAD_TO_REGISTER)
    { // If a target register is specified, load the unary
      // expression into the register and perform inline negation
        generate_expression(*unary_expr.expr, target_register);
        push_instruction(OpCode::NEG, {Operand(target_register)});
    }
    else
    { // If no target register is specified, allocate a register
      // and load the expression into that register, inline negate it and push it onto the stack
        RegId reg = allocate_register();
        generate_expression(*unary_expr.expr, reg);
        push_instruction(OpCode::NEG, {Operand(target_register)});
        push_instruction(OpCode::PUSHSTACK, {Operand(target_register)});
        free_register(reg);
    }
}

// Generates and emits a binary expression
void Generator::generate_binary_expression(BinaryExprNode bin_expr, RegId target_register) {}

// Generates and emits an index expression
void Generator::generate_index_expression(IndexExprNode idx_expr, RegId target_register)
{
    RegId target;
    RegId table = allocate_register(); // Allocate register to hold table
    RegId index = allocate_register(); // Allocate register to hold index

    // Determine destination register
    if (LOAD_TO_REGISTER)
        target = target_register;
    else
        target = allocate_register();

    // Generate table expression and store in allocated register
    generate_expression(*idx_expr.object, table);
    // Generate index expression and store in allocated register
    generate_expression(*idx_expr.index, index);

    // Emit the actual "indexing the table" instruction
    push_instruction(
        OpCode::GETTABLE,
        {
            Operand(target), // dst
            Operand(table),  // tbl
            Operand(index),  // idx
        }
    );

    // Free allocated holder registers
    free_register(table);
    free_register(index);

    // Check if the result needs to be pushed or not
    if (!LOAD_TO_REGISTER)
    {
        // Push result and free allocated holder register
        push_instruction(OpCode::PUSHSTACK, {Operand(target)});
        free_register(target);
    }
}

// Generates and emits a call expression
void Generator::generate_call_expression(CallExprNode call_expr, RegId target_register)
{
    // Load arguments
    for (ExprNode *arg : call_expr.args)
        // Pass in invalid register value so the compiler emits push instructions for the arguments
        // and automatically pushes them to the stack, setting it up automatically
        generate_expression(*arg, VIA_REGISTER_INVALID);

    RegId target;
    // Determine arg count by counting arguments
    TNumber argc = static_cast<TNumber>(call_expr.args.size());

    // Determine destination
    if (LOAD_TO_REGISTER)
        target = target_register;
    else
        target = allocate_register();

    // Generate and emit callee expression, load to target
    generate_expression(*call_expr.callee, target);
    // Emit call instruction
    push_instruction(
        OpCode::CALL,
        {
            Operand(target),
            Operand(argc),
        }
    );

    // Check if the result (return value 0) needs to be loaded to a register
    if (LOAD_TO_REGISTER)
        push_instruction(OpCode::POPSTACK, {Operand(target)});
    else // Free allocated register
        free_register(target);
}

// Generates and emits a variable expression
void Generator::generate_variable_expression(VarExprNode var_expr, RegId target_register)
{
    // Look for the symbol in the symbol-stack-offset map
    auto symbol_it = symbols.find(var_expr.ident.value);
    if (symbol_it != symbols.end())
    {
        RegId target;

        // Determine destination
        if (LOAD_TO_REGISTER)
            target = target_register;
        else
            target = allocate_register();

        // Emit variable retrieval instruction
        push_instruction(
            OpCode::GETSTACK,
            {
                Operand(target),
                Operand(static_cast<TNumber>(symbol_it->second)),
            }
        );

        // Check if the result needs to be loaded into a register or not
        if (!LOAD_TO_REGISTER)
        {
            // Emit push instruction
            push_instruction(OpCode::PUSHSTACK, {Operand(target)});
            // Free allocated register
            free_register(target);
        }
    }

    // Look for the symbol in the global table
    auto glob_it = std::find(built_in.begin(), built_in.end(), var_expr.ident.value);
    if (glob_it != built_in.end())
    {
        RegId target;

        // Determine destination register
        if (LOAD_TO_REGISTER)
            target = target_register;
        else
            target = allocate_register();

        char *ident_string = dupstring(var_expr.ident.value);
        cleaner.add_malloc(ident_string);

        // Emit global retriaval instruction
        push_instruction(
            OpCode::GETGLOBAL,
            {
                Operand(target),
                Operand(ident_string),
            }
        );

        // Check if the result needs to be stored in a register or not
        if (!LOAD_TO_REGISTER)
        {
            // Emit push instruction
            push_instruction(OpCode::PUSHSTACK, {Operand(target)});
            // Free allocated register
            free_register(target);
        }
    }

    // If the variable does not exit, replace it with nil
    // Check if the result needs to be stored in a register
    if (LOAD_TO_REGISTER)
        push_instruction(OpCode::LOADNIL, {Operand(target_register)});
    else
        push_instruction(OpCode::PUSHSTACK, {Operand()});
}

void Generator::generate_increment_expression(IncExprNode inc_expr, RegId target_register)
{
    RegId target;

    if (LOAD_TO_REGISTER)
        target = target_register;
    else
        target = allocate_register();

    generate_expression(*inc_expr.expr, target);
    push_instruction(OpCode::INCREMENT, {Operand(target)});

    if (!LOAD_TO_REGISTER)
    {
        push_instruction(OpCode::PUSHSTACK, {Operand(target)});
        free_register(target);
    }
}

void Generator::generate_decrement_expression(DecExprNode dec_expr, RegId target_register)
{
    RegId target;

    if (LOAD_TO_REGISTER)
        target = target_register;
    else
        target = allocate_register();

    generate_expression(*dec_expr.expr, target);
    push_instruction(OpCode::DECREMENT, {Operand(target)});

    if (!LOAD_TO_REGISTER)
    {
        push_instruction(OpCode::PUSHSTACK, {Operand(target)});
        free_register(target);
    }
}

void Generator::generate_expression(ExprNode expr, RegId target_register)
{
    if (LiteralExprNode *lit_expr = std::get_if<LiteralExprNode>(&expr.expr))
        generate_literal_expression(*lit_expr, target_register);
    else if (UnaryExprNode *un_expr = std::get_if<UnaryExprNode>(&expr.expr))
        generate_unary_expression(*un_expr, target_register);
    else if (BinaryExprNode *bin_expr = std::get_if<BinaryExprNode>(&expr.expr))
        generate_binary_expression(*bin_expr, target_register);
    else if (LambdaExprNode *lmd_expr = std::get_if<LambdaExprNode>(&expr.expr))
        generate_lambda_expression(*lmd_expr, target_register);
    else if (IndexExprNode *idx_expr = std::get_if<IndexExprNode>(&expr.expr))
        generate_index_expression(*idx_expr, target_register);
    else if (CallExprNode *call_expr = std::get_if<CallExprNode>(&expr.expr))
        generate_call_expression(*call_expr, target_register);
    else if (VarExprNode *var_expr = std::get_if<VarExprNode>(&expr.expr))
        generate_variable_expression(*var_expr, target_register);
    else if (IncExprNode *inc_expr = std::get_if<IncExprNode>(&expr.expr))
        generate_increment_expression(*inc_expr, target_register);
    else if (DecExprNode *dec_expr = std::get_if<DecExprNode>(&expr.expr))
        generate_decrement_expression(*dec_expr, target_register);
}

} // namespace via
