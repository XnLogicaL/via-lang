/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "gen.h"
#include "Parser/ast.h"
#include "builtin.h"
#include "instruction.h"
#include "optimize/bshift.h"
#include "optimize/constfold.h"

namespace via::Compilation
{

void compile_local_decl_stmt(Generator *, Parsing::AST::LocalDeclStmtNode);
void compile_global_decl_stmt(Generator *, Parsing::AST::GlobalDeclStmtNode);
void compile_func_decl_stmt(Generator *, Parsing::AST::FunctionDeclStmtNode);
void compile_call_stmt(Generator *, Parsing::AST::CallStmtNode);
void compile_assign_stmt(Generator *, Parsing::AST::AssignStmtNode);
void compile_while_stmt(Generator *, Parsing::AST::WhileStmtNode);
void compile_for_stmt(Generator *, Parsing::AST::ForStmtNode);
void compile_scope_stmt(Generator *, Parsing::AST::ScopeStmtNode);
void compile_if_stmt(Generator *, Parsing::AST::IfStmtNode);
void compile_switch_stmt(Generator *, Parsing::AST::SwitchStmtNode);
void compile_return_stmt(Generator *, Parsing::AST::ReturnStmtNode);
void compile_break_stmt(Generator *);
void compile_continue_stmt(Generator *);
void compile_statement(Generator *, Parsing::AST::StmtNode);

} // namespace via::Compilation
