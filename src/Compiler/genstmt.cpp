/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"

namespace via::Compilation
{

using namespace Parsing;
using namespace AST;

void Generator::generate_local_declaration_statement(LocalDeclStmtNode decl_stmt)
{
    viaRegister expr = generate_expression(*decl_stmt.value);
    register_pool.free_register(expr);

    push_instruction(
        OpCode::SETLOCAL,
        {
            viaC_newoperand(expr),
            viaC_newoperand(via_dupstring(decl_stmt.ident.value), true),
        }
    );
}

void Generator::generate_global_declaration_statement(GlobalDeclStmtNode decl_stmt)
{
    viaRegister expr = generate_expression(*decl_stmt.value);
    register_pool.free_register(expr);

    push_instruction(
        OpCode::SETGLOBAL,
        {
            viaC_newoperand(expr),
            viaC_newoperand(via_dupstring(decl_stmt.ident.value), true),
        }
    );
}

void Generator::generate_function_declaration_statement(FunctionDeclStmtNode func_stmt)
{
    /* Expected output:

        FUNC    Rx0
    [for each parameter]:
        POPARG  Rx1
        SETLOCAL Rx1 @<parameter-id>
    [body]:
        <function-body>...
        END
        SETLOCAL Rx0 @<function-id>

    */

    viaRegister dst = register_pool.allocate_register();

    push_instruction(OpCode::FUNC, {viaC_newoperand(dst)});

    for (TypedParamNode param : func_stmt.params)
    {
        viaRegister param_reg = register_pool.allocate_register();

        push_instruction(OpCode::POPARG, {viaC_newoperand(param_reg)});
        push_instruction(
            OpCode::SETLOCAL,
            {
                viaC_newoperand(param_reg),
                viaC_newoperand(via_dupstring(param.ident.value), true),
            }
        );

        register_pool.free_register(param_reg);
    }

    for (StmtNode stmt : func_stmt.body->statements)
        generate_statement(stmt);

    push_instruction(OpCode::END, {});
    push_instruction(
        func_stmt.is_global ? OpCode::SETGLOBAL : OpCode::SETLOCAL,
        {
            viaC_newoperand(dst),
            viaC_newoperand(via_dupstring(func_stmt.ident.value), true),
        }
    );

    register_pool.free_register(dst);
}

void Generator::generate_call_statement(CallStmtNode call_stmt)
{
    /* Expected output:

        LOAD    Rx0 <callee-expr>
    [for each argument]
        LOAD    Rx1 <argument-expr>
        PUSHARG Rx1
    [main]
        CALL Rx0

    */

    viaRegister callee = generate_expression(*call_stmt.callee);

    for (ExprNode arg : call_stmt.args)
    {
        viaRegister arg_reg = generate_expression(arg);
        register_pool.free_register(arg_reg);
        push_instruction(OpCode::PUSHARG, {viaC_newoperand(arg_reg)});
    }

    push_instruction(
        OpCode::CALL,
        {
            viaC_newoperand(callee),
            viaC_newoperand(static_cast<double>(call_stmt.args.size())),
        }
    );
}

void Generator::generate_assign_statement(AssignStmtNode asgn_stmt)
{
    /* Expected output:

        LOAD    Rx0 <expr>
    [if variable]:
        SETLOCAL Rx0 @<pid>
    [if index]:
        LOAD    Rx1 <table-expr>
        LOAD    Rx2 <index-expr>
        SETIDX  Rx1 Rx2 Rx0
    */

    viaRegister value = generate_expression(*asgn_stmt.value);

    if (VarExprNode *var_expr = std::get_if<VarExprNode>(asgn_stmt.target))
        push_instruction(
            OpCode::SETLOCAL,
            {
                viaC_newoperand(value),
                viaC_newoperand(via_dupstring(var_expr->ident.value), true),
            }
        );
    else if (IndexExprNode *idx_expr = std::get_if<IndexExprNode>(asgn_stmt.target))
    {
        viaRegister table = generate_expression(*idx_expr->object);
        viaRegister index = generate_expression(*idx_expr->index);

        push_instruction(
            OpCode::SETIDX,
            {
                viaC_newoperand(table),
                viaC_newoperand(index),
                viaC_newoperand(value),
            }
        );

        register_pool.free_register(table);
        register_pool.free_register(index);
    }
    else
        UNREACHABLE();

    register_pool.free_register(value);
}

void Generator::generate_while_statement(WhileStmtNode while_stmt)
{
    /* Expected output:

        LOAD    Rx true
        LABEL   @_LOOP_<pid>
        LOAD    Rx1 <cond>
        TOBOOL  Rx1 Rx1
        JLEQ    Rx1 Rx2 @_LOOP_BODY_<pid>
        JL      @_LOOP_EXIT_<pid>
        LABEL   @_LOOP_BODY_<pid>
        <loop-statements>...
        JL      @_LOOP_<pid>
        LABEL   @_LOOP_EXIT_<pid>

    */

    size_t loop_id = iota();
    std::string loop_head_label = via_dupstring(std::format("_LOOP_{}", loop_id));
    std::string loop_body_label = via_dupstring(std::format("_LOOP_BODY_{}", loop_id));
    std::string loop_exit_label = via_dupstring(std::format("_LOOP_EXIT_{}", loop_id));
    // Allocate register before head label to avoid reallocation
    viaRegister expect = register_pool.allocate_register();

    // Load the value to compare with condition, true
    push_instruction(
        OpCode::LOAD,
        {
            viaC_newoperand(expect),
            viaC_newoperand(true),
        }
    );

    push_instruction(
        OpCode::LABEL,
        {
            viaC_newoperand(via_dupstring(loop_head_label), true),
        }
    );

    viaRegister cond = generate_expression(*while_stmt.condition);

    push_instruction(
        OpCode::TOBOOL,
        {
            viaC_newoperand(cond),
            viaC_newoperand(cond),
        }
    );

    push_instruction(
        OpCode::JLEQ,
        {
            viaC_newoperand(cond),
            viaC_newoperand(expect),
            viaC_newoperand(via_dupstring(loop_body_label), true),
        }
    );

    push_instruction(
        OpCode::JL,
        {
            viaC_newoperand(via_dupstring(loop_exit_label), true),
        }
    );

    push_instruction(
        OpCode::LABEL,
        {
            viaC_newoperand(via_dupstring(loop_body_label), true),
        }
    );

    for (StmtNode stmt : while_stmt.body->statements)
        generate_statement(stmt);

    push_instruction(
        OpCode::JL,
        {
            viaC_newoperand(via_dupstring(loop_head_label), true),
        }
    );

    push_instruction(
        OpCode::LABEL,
        {
            viaC_newoperand(via_dupstring(loop_exit_label), true),
        }
    );

    register_pool.free_register(cond);
    register_pool.free_register(expect);
}

// TODO
void Generator::generate_for_statement(ForStmtNode) {}
void Generator::generate_scope_statement(ScopeStmtNode) {}

void Generator::generate_if_statement(IfStmtNode if_stmt)
{
    /* Expected output:

        LOAD    Rx0 false
        LOAD    Rx1 <if-condition-expr>
        TOBOOL  Rx1 Rx1
        JLEQ    Rx1 Rx0 @_IF_EXIT_<pid>
        <if-body-statements>...
        JL      @_IF_EXIT_<pid>
    [for each elseif block]:
        LOAD    Rx2 <elseif-condition-expr>
        TOBOOL  Rx2 Rx2
        JLEQ    Rx2 Rx0 @_IF_EXIT_<pid>
        <elseif-body-statements>...
        JL      @_IF_EXIT_<pid>
    [else block]:
        <else-body-statements>...
    [exit point]:
        LABEL   @_IF_EXIT_<pid>

    */

    size_t if_id = iota();
    std::string if_exit_label = std::format("_IF_EXIT_{}", if_id);

    viaRegister test_reg = register_pool.allocate_register();

    push_instruction(
        OpCode::LOAD,
        {
            viaC_newoperand(test_reg),
            viaC_newoperand(false),
        }
    );

    viaRegister cond_reg = generate_expression(*if_stmt.condition);
    register_pool.free_register(cond_reg);

    push_instruction(
        OpCode::TOBOOL,
        {
            viaC_newoperand(cond_reg),
            viaC_newoperand(cond_reg),
        }
    );

    push_instruction(
        OpCode::JLEQ,
        {
            viaC_newoperand(cond_reg),
            viaC_newoperand(test_reg),
            viaC_newoperand(via_dupstring(if_exit_label), true),
        }
    );

    for (StmtNode if_body_stmt : if_stmt.then_body->statements)
        generate_statement(if_body_stmt);

    for (ElifStmtNode elif_stmt : if_stmt.elif_branches)
    {
        viaRegister cond_reg = generate_expression(*elif_stmt.condition);
        register_pool.free_register(cond_reg);

        push_instruction(
            OpCode::TOBOOL,
            {
                viaC_newoperand(cond_reg),
                viaC_newoperand(cond_reg),
            }
        );

        push_instruction(
            OpCode::JLEQ,
            {
                viaC_newoperand(cond_reg),
                viaC_newoperand(test_reg),
                viaC_newoperand(via_dupstring(if_exit_label), true),
            }
        );

        for (StmtNode elif_body_stmt : elif_stmt.body->statements)
            generate_statement(elif_body_stmt);
    }

    if (if_stmt.else_body.has_value())
        for (StmtNode else_body_stmt : if_stmt.else_body->statements)
            generate_statement(else_body_stmt);

    push_instruction(
        OpCode::LABEL,
        {
            viaC_newoperand(via_dupstring(if_exit_label), true),
        }
    );

    register_pool.free_register(test_reg);
}

// TODO
void Generator::generate_switch_statement(SwitchStmtNode) {}

void Generator::generate_return_statement(ReturnStmtNode ret_stmt)
{
    /* Expected output:

    [for each return value]:
        LOAD    Rx1 <ret-value-expr>
        PUSHRET Rx1
    [body]:
        RET

    */

    for (ExprNode ret_value : ret_stmt.values)
    {
        viaRegister ret_value_reg = generate_expression(ret_value);
        push_instruction(OpCode::PUSHRET, {viaC_newoperand(ret_value_reg)});
        register_pool.free_register(ret_value_reg);
    }

    push_instruction(OpCode::RET, {});
}

void Generator::generate_break_statement()
{
    /* Expected output:

        JL      @_LOOP_EXIT_<current-pid>

    */

    std::string loop_exit_label = std::format("_LOOP_EXIT_{}", __iota__);

    push_instruction(
        OpCode::JL,
        {
            viaC_newoperand(via_dupstring(loop_exit_label), true),
        }
    );
}

void Generator::generate_continue_statement()
{
    /* Expected output:

       JL      @_LOOP_<current-pid>

   */

    std::string loop_head_label = std::format("_LOOP_{}", __iota__);

    push_instruction(
        OpCode::JL,
        {
            viaC_newoperand(via_dupstring(loop_head_label), true),
        }
    );
}

void Generator::generate_statement(Parsing::AST::StmtNode stmt)
{
    if (auto *local_decl = std::get_if<Parsing::AST::LocalDeclStmtNode>(&stmt))
        generate_local_declaration_statement(*local_decl);
    else if (auto *global_decl = std::get_if<Parsing::AST::GlobalDeclStmtNode>(&stmt))
        generate_global_declaration_statement(*global_decl);
    else if (auto *func_decl = std::get_if<Parsing::AST::FunctionDeclStmtNode>(&stmt))
        generate_function_declaration_statement(*func_decl);
    else if (auto *call_stmt = std::get_if<Parsing::AST::CallStmtNode>(&stmt))
        generate_call_statement(*call_stmt);
    else if (auto *assign_stmt = std::get_if<Parsing::AST::AssignStmtNode>(&stmt))
        generate_assign_statement(*assign_stmt);
    else if (auto *while_stmt = std::get_if<Parsing::AST::WhileStmtNode>(&stmt))
        generate_while_statement(*while_stmt);
    else if (auto *for_stmt = std::get_if<Parsing::AST::ForStmtNode>(&stmt))
        generate_for_statement(*for_stmt);
    else if (auto *scope_stmt = std::get_if<Parsing::AST::ScopeStmtNode>(&stmt))
        generate_scope_statement(*scope_stmt);
    else if (auto *if_stmt = std::get_if<Parsing::AST::IfStmtNode>(&stmt))
        generate_if_statement(*if_stmt);
    else if (auto *switch_stmt = std::get_if<Parsing::AST::SwitchStmtNode>(&stmt))
        generate_switch_statement(*switch_stmt);
    else if (auto *return_stmt = std::get_if<Parsing::AST::ReturnStmtNode>(&stmt))
        generate_return_statement(*return_stmt);
    else if (auto *break_stmt = std::get_if<Parsing::AST::BreakStmtNode>(&stmt))
        generate_break_statement();
    else if (auto *continue_stmt = std::get_if<Parsing::AST::ContinueStmtNode>(&stmt))
        generate_continue_statement();
    else
        UNREACHABLE();
}

} // namespace via::Compilation