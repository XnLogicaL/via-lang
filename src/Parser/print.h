#ifndef AST_PRINTER_H
#define AST_PRINTER_H

#include "common.h"
#include "ast.h"
#include "../Lexer/token.h"

namespace via
{

namespace Parsing
{

namespace AST
{

void print_token(const Token& token);
void print_type_node(const TypeNode* type);
void print_expr_node(const ExprNode* expr);
void print_typed_param_stmt_node(const TypedParamStmtNode& node);
void print_local_decl_stmt_node(const LocalDeclStmtNode& node);
void print_glob_decl_stmt_node(const GlobDeclStmtNode& node);
void print_call_stmt_node(const CallStmtNode& node);
void print_return_stmt_node(const ReturnStmtNode& node);
void print_scope_stmt_node(const ScopeStmtNode& scope);
void print_stmt_node(StmtNode* node);
void print_ast(AST* ast);
    
} // namespace AST

} // namespace Parsing

} // namespace via

#endif // AST_PRINTER_H
