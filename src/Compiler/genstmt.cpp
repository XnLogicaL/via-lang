/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"

namespace via
{

void Generator::generate_variable_declaration_statement(VariableDeclStmtNode decl_stmt)
{
    stack_pointer++;

    RegId dst = allocate_register();
    TNumber sp = stack_pointer;

    generate_expression(*decl_stmt.value, dst);
    push_instruction(OpCode::SETSTACK, {Operand(dst), Operand(sp)});
}

void Generator::generate_function_declaration_statement(FunctionDeclStmtNode func_stmt)
{
    // TODO: Account for global functions
    stack_pointer++;

    RegId dst = allocate_register();
    push_instruction(OpCode::LOADFUNCTION, {Operand(dst)});

    for (StmtNode stmt : func_stmt.body->statements)
        generate_statement(stmt);

    if (program.bytecode->instructions.back().op != OpCode::RETURN)
        push_instruction(OpCode::RETURN, {});

    push_instruction(OpCode::SETSTACK, {Operand(dst), Operand(static_cast<TNumber>(stack_pointer))});
    free_register(dst);
}

void Generator::generate_call_statement(CallStmtNode call_stmt)
{
    bool is_methodcall = std::get_if<IndexExprNode>(&call_stmt.callee->expr);
    RegId callee = allocate_register();
    TNumber argc = static_cast<TNumber>(call_stmt.args.size() + is_methodcall);

    // Check if the self argument is required
    if (is_methodcall)
        push_instruction(OpCode::PUSHSTACK, {Operand(callee)});

    for (ExprNode *arg : call_stmt.args)
        // Automatically push the arguments onto the stack
        generate_expression(*arg, VIA_REGISTER_INVALID);

    generate_expression(*call_stmt.callee, callee);
    push_instruction(OpCode::CALL, {Operand(callee), Operand(argc)});
    free_register(callee);
}

void Generator::generate_assign_statement(AssignStmtNode asgn_stmt)
{
    if (VarExprNode *var_expr = std::get_if<VarExprNode>(&asgn_stmt.target->expr))
    {
        auto it = symbols.find(var_expr->ident.value);
        if (it != symbols.end())
        {
            RegId val = allocate_register();
            LocalId stk_offset = it->second;

            generate_expression(*asgn_stmt.value, val);
            push_instruction(OpCode::SETSTACK, {Operand(stk_offset), Operand(val)});
            free_register(val);
        }
    }
    // TODO: Add support for other assignable expressions, such as index expressions
}

void Generator::generate_while_statement(WhileStmtNode while_stmt)
{
    RegId cond = allocate_register();
    TNumber exit_label = while_stmt.body->statements.size();
    size_t head_offset = program.bytecode->instructions.size();

    generate_expression(*while_stmt.condition, cond);
    push_instruction(
        OpCode::JUMPIFNOT,
        {
            Operand(cond),
            Operand(exit_label + 1), // +1 to skip the tail jump instruction!
        }
    );

    for (StmtNode stmt : while_stmt.body->statements)
        generate_statement(stmt);

    TNumber head_label = -(program.bytecode->instructions.size() - head_offset);

    push_instruction(OpCode::JUMP, {Operand(head_label)});
}

void Generator::generate_for_statement(ForStmtNode for_stmt) {}

void Generator::generate_scope_statement(ScopeStmtNode scope_stmt) {}

void Generator::generate_if_statement(IfStmtNode if_stmt) {}

void Generator::generate_switch_statement(SwitchStmtNode switch_stmt) {}

void Generator::generate_return_statement(ReturnStmtNode ret_stmt) {}

void Generator::generate_break_statement() {}

void Generator::generate_continue_statement() {}

void Generator::generate_statement(StmtNode stmt)
{
    initialize_with_chunk = true;
    current_chunk = new Chunk();
    current_chunk->mcode = nullptr;
    current_chunk->pc = 0;

    if (VariableDeclStmtNode *var_decl = std::get_if<VariableDeclStmtNode>(&stmt.stmt))
        generate_variable_declaration_statement(*var_decl);
    else if (FunctionDeclStmtNode *func_decl = std::get_if<FunctionDeclStmtNode>(&stmt.stmt))
        generate_function_declaration_statement(*func_decl);
    else if (CallStmtNode *call_stmt = std::get_if<CallStmtNode>(&stmt.stmt))
        generate_call_statement(*call_stmt);
    else if (AssignStmtNode *assign_stmt = std::get_if<AssignStmtNode>(&stmt.stmt))
        generate_assign_statement(*assign_stmt);
    else if (WhileStmtNode *while_stmt = std::get_if<WhileStmtNode>(&stmt.stmt))
        generate_while_statement(*while_stmt);
    else if (ForStmtNode *for_stmt = std::get_if<ForStmtNode>(&stmt.stmt))
        generate_for_statement(*for_stmt);
    else if (ScopeStmtNode *scope_stmt = std::get_if<ScopeStmtNode>(&stmt.stmt))
        generate_scope_statement(*scope_stmt);
    else if (IfStmtNode *if_stmt = std::get_if<IfStmtNode>(&stmt.stmt))
        generate_if_statement(*if_stmt);
    else if (SwitchStmtNode *switch_stmt = std::get_if<SwitchStmtNode>(&stmt.stmt))
        generate_switch_statement(*switch_stmt);
    else if (ReturnStmtNode *return_stmt = std::get_if<ReturnStmtNode>(&stmt.stmt))
        generate_return_statement(*return_stmt);
    else if (std::get_if<BreakStmtNode>(&stmt.stmt))
        generate_break_statement();
    else if (std::get_if<ContinueStmtNode>(&stmt.stmt))
        generate_continue_statement();
}

} // namespace via