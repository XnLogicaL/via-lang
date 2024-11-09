#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "common.h"
#include "ast.h"
#include "token.h"

namespace via
{

namespace Parsing
{

namespace AST
{

std::string stringify_type_node(const TypeNode& type);
std::string stringify_expr_node(const ExprNode& expr);
std::string stringify_stmt_node(const StmtNode& node);
std::string stringify_typed_param_stmt_node(const TypedParamStmtNode node);
std::string stringify_local_decl_stmt_node(const LocalDeclStmtNode node);
std::string stringify_glob_decl_stmt_node(const GlobDeclStmtNode node);
std::string stringify_call_stmt_node(const CallStmtNode node);
std::string stringify_return_stmt_node(const ReturnStmtNode node);
std::string stringify_scope_stmt_node(const ScopeStmtNode scope);
std::string stringify_ast(const AST ast);
    
} // namespace AST

} // namespace Parsing

} // namespace via

#endif // AST_PRINTER_H
