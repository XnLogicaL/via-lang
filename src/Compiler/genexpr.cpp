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
        // If a target register is specified, use the LOAD instruction to load the
        // value into the register
        load_operand(target_register, operand);
    else
        // If no target register is specified, push the value onto the stack
        push_instruction(OpCode::PUSH, {Operand(operand)});
}

// Generates and emits a unary expression
void Generator::generate_unary_expression(UnaryExprNode unary_expr, RegId target_register)
{
    if (LOAD_TO_REGISTER)
    { // If a target register is specified, load the unary
      // expression into the register and perform inline negation
        generate_expression(*unary_expr.expr, target_register);
        push_instruction(OpCode::NEGI, {Operand(target_register)});
    }
    else
    { // If no target register is specified, allocate a register
      // and load the expression into that register, inline negate it and push it onto the stack
        RegId reg = allocate_register();
        generate_expression(*unary_expr.expr, reg);
        push_instruction(OpCode::NEGI, {Operand(target_register)});
        push_instruction(OpCode::PUSH, {Operand(target_register)});
        free_register(reg);
    }
}

// Generates and emits a binary expression
void Generator::generate_binary_expression(BinaryExprNode bin_expr, RegId target_register)
{
    // Simple operators with no context-specific opcodes, only R-R-R opcodes
    static const HashMap<TokenType, OpCode> simple_operator_map = {
        {TokenType::OP_LT, OpCode::LT},
        {TokenType::OP_GT, OpCode::GT},
        {TokenType::OP_EQ, OpCode::EQ},
        {TokenType::OP_NEQ, OpCode::NEQ},
        {TokenType::OP_LEQ, OpCode::LE},
        {TokenType::OP_GEQ, OpCode::GE},
    };

    // Complex operators with context-specific opcodes, 6 combinations of operand types for each
    static const HashMap<TokenType, std::array<OpCode, 6>> complex_operator_map = {
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

    OpCode op;                    // Opcode of the operation
    Operand dst(target_register); // Destination register
    Operand lhs, rhs;             // lhs and rhs holder register operands
    uint8_t complex_id;           // Complex id, aka the offset from the R-R-R opcode to the I-N opcode
    RegId lhs_register = allocate_register();
    RegId rhs_register = allocate_register();

    // Look for the counterpart opcode for the operator
    auto it = simple_operator_map.find(bin_expr.op.type);
    auto it_complex = complex_operator_map.find(bin_expr.op.type);

    // Check if the operator is simple or not
    if (it == simple_operator_map.end())
    {
        // For checking if the lhs or rhs expression is literal
        LiteralExprNode *lhs_literal = std::get_if<LiteralExprNode>(&bin_expr.lhs->expr);
        LiteralExprNode *rhs_literal = std::get_if<LiteralExprNode>(&bin_expr.rhs->expr);

        if (lhs_literal && rhs_literal)
        { // <OP>NN
            complex_id = 3;
            // Generate literal operands
            lhs = generate_operand(*lhs_literal);
            rhs = generate_operand(*rhs_literal);
        }
        else if (rhs_literal)
        { // <OP>RN
            complex_id = 1;
            // Generate lhs
            generate_expression(*bin_expr.lhs, lhs_register);
            // Generate rhs
            rhs = generate_operand(*rhs_literal);
            lhs = lhs_register;
        }
        else if (lhs_literal)
        { // <OP>NR
            complex_id = 2;
            // Generate lhs
            lhs = generate_operand(*lhs_literal);
            rhs = rhs_register;
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
            lhs = lhs_register;
            rhs = rhs_register;
        }

        // Set the opcode to complex opcode
        op = it_complex->second.at(complex_id);
    }
    else // Set opcode to primitive opcode
        op = it->second;

    // Check if the result needs to be pushed or not
    if (LOAD_TO_REGISTER)
    {
        // Check if OpCode is an inline arithmetic opcode
        if (complex_id > 3)
        { // If it is, load the literal into a register before emitting inline opcode
            load_operand(dst, lhs);
            push_instruction(op, {Operand(dst), Operand(rhs)});
            return;
        }

        // Emit the binary instruction and free the holder registers
        push_instruction(op, {Operand(dst), Operand(lhs), Operand(rhs)});
        free_register(lhs_register);
        free_register(rhs_register);
    }
    else
    {
        // Allocate temporary register for the result
        RegId temp_register = allocate_temp_register();
        Operand temp = temp_register;

        // Push the result
        push_instruction(op, {Operand(temp), Operand(lhs), Operand(rhs)});
        push_instruction(OpCode::PUSH, {Operand(temp)});
    }
}

// Generates and emits a lambda expression
void Generator::generate_lambda_expression(LambdaExprNode lmd_expr, RegId target_register)
{
    RegId destination = target_register;
    // Determine destination register
    if (!LOAD_TO_REGISTER)
        destination = allocate_register();

    // Push function load instruction
    push_instruction(OpCode::LOADFUNCTION, {Operand(destination)});

    // Generate statements
    for (StmtNode lambda_stmt : lmd_expr.body->statements)
        generate_statement(lambda_stmt);

    // Check if the lambda body has been terminated by a RET instruction
    // If not, insert one
    if (program.bytecode->instructions.back().op != OpCode::RET)
        push_instruction(OpCode::RET, {});

    // Check if the function is supposed to be pushed onto the stack
    if (!LOAD_TO_REGISTER)
    { // Push the function onto the stack
      // and free the temporary holder register
        push_instruction(OpCode::PUSH, {Operand(destination)});
        free_register(destination);
    }
}

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
        push_instruction(OpCode::PUSH, {Operand(target)});
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
        push_instruction(OpCode::POP, {Operand(target)});
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
            push_instruction(OpCode::PUSH, {Operand(target)});
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
            push_instruction(OpCode::PUSH, {Operand(target)});
            // Free allocated register
            free_register(target);
        }
    }

    // If the variable does not exit, replace it with nil
    // Check if the result needs to be stored in a register
    if (LOAD_TO_REGISTER)
        push_instruction(OpCode::LOADNIL, {Operand(target_register)});
    else
        push_instruction(OpCode::PUSH, {Operand()});
}

void Generator::generate_increment_expression(IncExprNode inc_expr, RegId target_register)
{
    RegId target;

    if (LOAD_TO_REGISTER)
        target = target_register;
    else
        target = allocate_register();

    generate_expression(*inc_expr.expr, target);
    push_instruction(OpCode::INC, {Operand(target)});

    if (!LOAD_TO_REGISTER)
    {
        push_instruction(OpCode::PUSH, {Operand(target)});
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
    push_instruction(OpCode::DEC, {Operand(target)});

    if (!LOAD_TO_REGISTER)
    {
        push_instruction(OpCode::PUSH, {Operand(target)});
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
