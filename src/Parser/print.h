/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include "common.h"
#include "ast.h"

namespace via::Parsing::AST
{

std::string stringify_type_node(const TypeNode &type);
std::string stringify_expr_node(const ExprNode &expr);
std::string stringify_stmt_node(const StmtNode &node);
std::string stringify_typed_param_stmt_node(const TypedParamNode node);
std::string stringify_local_decl_stmt_node(const LocalDeclStmtNode node);
std::string stringify_glob_decl_stmt_node(const GlobalDeclStmtNode node);
std::string stringify_call_stmt_node(const CallStmtNode node);
std::string stringify_return_stmt_node(const ReturnStmtNode node);
std::string stringify_scope_stmt_node(const ScopeStmtNode scope);
std::string stringify_ast(const AST ast);

} // namespace via::Parsing::AST
