/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "vstatement.h"
#include "vexpression.h"

namespace via::Compilation
{

using namespace Parsing;
using namespace AST;

void compile_local_decl_stmt(Generator *gen, LocalDeclStmtNode decl)
{
    viaRegister value = compile_expression(gen, decl.value.value());

    gen->free_register(value);
    gen->pushinstr(viaC_newinstruction(
        "SETLOCAL",
        {
            viaC_newoperand(value),
            viaC_newoperand(via_dupstring(decl.ident.value), true),
        }
    ));
}

void compile_global_decl_stmt(Generator *gen, GlobalDeclStmtNode decl)
{
    viaRegister value = compile_expression(gen, decl.value.value());

    gen->free_register(value);
    gen->pushinstr(viaC_newinstruction(
        "SETGLOBAL",
        {
            viaC_newoperand(value),
            viaC_newoperand(via_dupstring(decl.ident.value), true),
        }
    ));
}

void compile_func_decl_stmt(Generator *gen, FunctionDeclStmtNode decl)
{
    viaRegister func = gen->get_available_register();

    gen->pushinstr(viaC_newinstruction(
        "FUNC",
        {
            viaC_newoperand(func),
        }
    ));

    for (StmtNode stmt : decl.body->statements)
        compile_statement(gen, stmt);

    gen->free_register(func);
    gen->pushinstr(viaC_newinstruction("END", {}));
    gen->pushinstr(viaC_newinstruction(
        decl.is_global ? "SETGLOBAL" : "SETLOCAL",
        {
            viaC_newoperand(func),
            viaC_newoperand(via_dupstring(decl.ident.value), true),
        }
    ));
}

void compile_call_stmt(Generator *gen, CallStmtNode call)
{
    size_t argc = call.args.size();
    viaRegister callee = compile_expression(gen, *call.callee);

    for (ExprNode arg : call.args)
    {
        viaRegister arg_reg = compile_expression(gen, arg);
        gen->free_register(arg_reg);
        gen->pushinstr(viaC_newinstruction(
            "PUSHARG",
            {
                viaC_newoperand(arg_reg),
            }
        ));
    }

    gen->free_register(callee);
    gen->pushinstr(viaC_newinstruction(
        "CALL",
        {
            viaC_newoperand(callee),
            viaC_newoperand(static_cast<double>(argc)),
        }
    ));
}

void compile_assign_stmt(Generator *gen, Parsing::AST::AssignStmtNode asgn)
{
    viaRegister value = compile_expression(gen, *asgn.value);

    if (VarExprNode *var = std::get_if<VarExprNode>(asgn.target))
    {
        gen->free_register(value);
        gen->pushinstr(viaC_newinstruction(
            "SETLOCAL",
            {
                viaC_newoperand(via_dupstring(var->ident.value), true),
                viaC_newoperand(value),
            }
        ));
    }
    else if (IndexExprNode *idx = std::get_if<IndexExprNode>(asgn.target))
    {
        viaRegister idx_reg = compile_expression(gen, *idx->index);

        gen->free_register(value);
        gen->free_register(idx_reg);
        gen->pushinstr(viaC_newinstruction(
            "SETIDX",
            {
                viaC_newoperand(value),
                viaC_newoperand(idx_reg),
            }
        ));
    }
}

void compile_while_stmt(Generator *gen, Parsing::AST::WhileStmtNode stmt)
{
    size_t uuid = gen->uuid();
    std::string loop_lbl = std::format("LC{}", uuid);
    std::string esc_lbl = std::format("ESC{}", uuid);

    gen->pushinstr(viaC_newinstruction(
        "LABEL",
        {
            viaC_newoperand(via_dupstring(loop_lbl)),
        }
    ));

    viaRegister cond = compile_expression(gen, *stmt.condition);
    viaRegister cmp = gen->get_available_register();

    gen->pushinstr(viaC_newinstruction(
        "LOAD",
        {
            viaC_newoperand(cmp),
            viaC_newoperand(true),
        }
    ));

    gen->pushinstr(viaC_newinstruction(
        "TOBOOL",
        {
            viaC_newoperand(cond),
            viaC_newoperand(cond),
        }
    ));

    gen->pushinstr(viaC_newinstruction(
        "JLNEQ",
        {
            viaC_newoperand(cond),
            viaC_newoperand(cmp),
            viaC_newoperand(via_dupstring(esc_lbl), true),
        }
    ));

    for (StmtNode loop_stmt : stmt.body->statements)
        compile_statement(gen, loop_stmt);

    gen->pushinstr(viaC_newinstruction(
        "JL",
        {
            viaC_newoperand(via_dupstring(loop_lbl), true),
        }
    ));

    gen->pushinstr(viaC_newinstruction(
        "LABEL",
        {
            viaC_newoperand(via_dupstring(esc_lbl), true),
        }
    ));
}

void compile_for_stmt(Generator *, ForStmtNode) {}
void compile_scope_stmt(Generator *, Parsing::AST::ScopeStmtNode) {}
void compile_if_stmt(Generator *, Parsing::AST::IfStmtNode) {}
void compile_switch_stmt(Generator *, Parsing::AST::SwitchStmtNode) {}

void compile_return_stmt(Generator *gen, Parsing::AST::ReturnStmtNode ret)
{
    for (ExprNode val : ret.values)
    {
        viaRegister reg = compile_expression(gen, val);
        gen->free_register(reg);
        gen->pushinstr(viaC_newinstruction(
            "PUSHRET",
            {
                viaC_newoperand(reg),
            }
        ));
    }
}

void compile_break_stmt(Generator *) {}
void compile_continue_stmt(Generator *) {}

void compile_statement(Generator *gen, StmtNode stmt)
{
#define CHECK(type, id) type *id = std::get_if<type>(&stmt)
    if (CHECK(LocalDeclStmtNode, decl))
        compile_local_decl_stmt(gen, *decl);
    else if (CHECK(GlobalDeclStmtNode, decl))
        compile_global_decl_stmt(gen, *decl);
    else if (CHECK(FunctionDeclStmtNode, decl))
        compile_func_decl_stmt(gen, *decl);
    else if (CHECK(CallStmtNode, call))
        compile_call_stmt(gen, *call);
    else if (CHECK(AssignStmtNode, asgn))
        compile_assign_stmt(gen, *asgn);
    else if (CHECK(WhileStmtNode, whiles))
        compile_while_stmt(gen, *whiles);
    else if (CHECK(ForStmtNode, fors))
        compile_for_stmt(gen, *fors);
    else if (CHECK(ScopeStmtNode, scope))
        compile_scope_stmt(gen, *scope);
    else if (CHECK(IfStmtNode, ifs))
        compile_if_stmt(gen, *ifs);
    else if (CHECK(SwitchStmtNode, switchs))
        compile_switch_stmt(gen, *switchs);
    else if (CHECK(ReturnStmtNode, ret))
        compile_return_stmt(gen, *ret);
    else if (CHECK(BreakStmtNode, _))
        compile_break_stmt(gen);
    else if (CHECK(ContinueStmtNode, _))
        compile_continue_stmt(gen);
#undef CHECK
}

} // namespace via::Compilation