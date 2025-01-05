/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#include "gen.h"

namespace via::Compilation
{

using namespace Parsing;
using namespace AST;

void Generator::generate_local_declaration_statement(LocalDeclStmtNode decl_stmt) {}

void Generator::generate_global_declaration_statement(GlobalDeclStmtNode decl_stmt) {}

void Generator::generate_function_declaration_statement(FunctionDeclStmtNode func_stmt) {}

void Generator::generate_call_statement(CallStmtNode call_stmt) {}

void Generator::generate_assign_statement(AssignStmtNode asgn_stmt) {}

void Generator::generate_while_statement(WhileStmtNode while_stmt) {}

// TODO
void Generator::generate_for_statement(ForStmtNode) {}
void Generator::generate_scope_statement(ScopeStmtNode) {}

void Generator::generate_if_statement(IfStmtNode if_stmt) {}

// TODO
void Generator::generate_switch_statement(SwitchStmtNode) {}

void Generator::generate_return_statement(ReturnStmtNode ret_stmt) {}

void Generator::generate_break_statement() {}

void Generator::generate_continue_statement() {}

void Generator::generate_statement(Parsing::AST::StmtNode stmt)
{
    initialize_with_chunk = true;
    current_chunk = new Chunk;
    current_chunk->mcode = nullptr;
    current_chunk->pc = 0;

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
    else if (std::get_if<Parsing::AST::BreakStmtNode>(&stmt))
        generate_break_statement();
    else if (std::get_if<Parsing::AST::ContinueStmtNode>(&stmt))
        generate_continue_statement();
    else
        UNREACHABLE();
}

} // namespace via::Compilation