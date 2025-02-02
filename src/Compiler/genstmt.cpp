#include "gen.h"

namespace via
{

void Generator::generate_variable_declaration_statement(VariableDeclStmtNode decl_stmt)
{
    if (decl_stmt.decl_type == DeclarationType::Local)
    {
        RegId dst = allocate_register();

        if (is_constexpr(*decl_stmt.value))
        {
            ExprNode const_expr = evaluate_constexpr(*decl_stmt.value);
            LiteralExprNode const_lit = std::get<LiteralExprNode>(const_expr.expr);
            size_t const_idx = load_constant(const_lit);
            TNumber const_id = static_cast<TNumber>(const_idx);

            push_instruction(OpCode::PUSHK, {Operand(const_id)});
            add_bc_info(std::format("local {} = {}", decl_stmt.ident.value, const_lit.value.value));
        }
        else
        {
            generate_expression(*decl_stmt.value, dst);
            push_instruction(OpCode::PUSH, {Operand(dst)});
            add_bc_info(std::format("local {} = <non-constexpr>", decl_stmt.ident.value));
        }

        stack.push(decl_stmt.ident.value);
        free_register(dst);
    }
}

void Generator::generate_function_declaration_statement(FunctionDeclStmtNode func_stmt)
{
    RegId dst = allocate_register();
    push_instruction(OpCode::LOADFUNCTION, {Operand(dst)});
    add_bc_info(std::format("func {}", func_stmt.ident.value));

    for (TypedParamNode param : func_stmt.params)
        stack.push(param.ident.value);

    generate_scope_statement(*func_stmt.body, true);

    if (program.bytecode->instructions.back().op != OpCode::RETURN)
        push_instruction(OpCode::RETURN, {});

    if (func_stmt.decl_type == DeclarationType::Local)
    {
        push_instruction(OpCode::PUSH, {Operand(dst)});
        free_register(dst);
    }
}

void Generator::generate_call_statement(CallStmtNode call_stmt)
{
    bool is_methodcall = std::get_if<IndexExprNode>(&call_stmt.callee->expr);
    RegId callee = allocate_register();
    TNumber argc = static_cast<TNumber>(call_stmt.args.size() + is_methodcall);

    generate_expression(*call_stmt.callee, callee);

    if (is_methodcall)
    {
        push_instruction(OpCode::PUSH, {Operand(callee)});
        add_bc_info("self");
    }

    for (ExprNode *arg : call_stmt.args)
    {
        if (is_constexpr(*arg))
        {
            ExprNode const_expr = evaluate_constexpr(*arg);
            LiteralExprNode const_lit = std::get<LiteralExprNode>(const_expr.expr);
            size_t const_idx = load_constant(const_lit);
            TNumber const_id = static_cast<TNumber>(const_idx);

            push_instruction(OpCode::PUSHK, {Operand(const_id)});
            add_bc_info(const_lit.value.value);
        }
        else
        {
            generate_expression(*arg, VIA_REGISTER_INVALID);
        }
    }

    push_instruction(OpCode::CALL, {Operand(callee), Operand(argc)});
    free_register(callee);
}

void Generator::generate_assign_statement(AssignStmtNode asgn_stmt)
{
    if (VarExprNode *var_expr = std::get_if<VarExprNode>(&asgn_stmt.target->expr))
    {
    }
}

void Generator::generate_while_statement(WhileStmtNode while_stmt)
{
    RegId cond = allocate_register();
    size_t loop_start = program.bytecode->instructions.size();

    generate_expression(*while_stmt.condition, cond);
    push_instruction(OpCode::JUMPIFNOT, {Operand(cond), Operand(0.0f)});

    size_t body_start = program.bytecode->instructions.size();
    generate_scope_statement(*while_stmt.body, false);

    size_t loop_end = program.bytecode->instructions.size();

    Instruction &jump_if_not_instr = program.bytecode->instructions.at(loop_start);
    jump_if_not_instr.operand2 = Operand(TNumber(loop_end - loop_start - 1));

    push_instruction(OpCode::JUMP, {Operand(TNumber(loop_start - loop_end))});
}

void Generator::generate_for_statement(ForStmtNode for_stmt) {}

void Generator::generate_scope_statement(ScopeStmtNode scope_stmt, bool is_function)
{
    saved_stack_pointer = stack.size();

    for (StmtNode stmt : scope_stmt.statements)
        generate_statement(stmt);

    // Check if the scope body belongs to a scope
    // If it does, then manually clean up all variables pushed onto the stack by the scope
    if (!is_function)
    {
        size_t sp_diff = stack.size() - saved_stack_pointer;
        for (size_t i = 0; i < sp_diff; i++)
        {
            stack.pop();
            push_instruction(OpCode::POP, {Operand(RegId(VIA_REGISTER_COUNT))});
        }
    }
}

void Generator::generate_if_statement(IfStmtNode if_stmt) {}

void Generator::generate_switch_statement(SwitchStmtNode switch_stmt) {}

void Generator::generate_return_statement(ReturnStmtNode ret_stmt)
{
    size_t retc = ret_stmt.values.size();
    for (ExprNode ret : ret_stmt.values)
        generate_expression(ret, VIA_REGISTER_INVALID);

    push_instruction(OpCode::RETURN, {Operand(static_cast<TNumber>(retc))});
}

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
        generate_scope_statement(*scope_stmt, false);
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
