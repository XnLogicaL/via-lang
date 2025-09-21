/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <mimalloc.h>
#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
#include "diagnostics.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "support/memory.h"

namespace via {

class Parser final
{
  public:
    Parser(const std::string& source, const TokenTree& ttree, DiagContext& diag)
        : m_source(source),
          m_cursor(ttree.cbegin().base()),
          m_diags(diag)
    {}

  public:
    ScopedAllocator& get_allocator() { return m_alloc; }
    SyntaxTree parse();

  private:
    bool match(TokenKind kind, int ahead = 0);
    bool optional(TokenKind kind);

    const Token* peek(int ahead = 0);
    const Token* advance();
    const Token* expect(TokenKind kind, const char* task);

    // Special
    const ast::Path* parse_static_path();
    const ast::Expr* parse_lvalue();
    const ast::Parameter* parse_parameter();
    const ast::AttributeGroup* parse_attributes();

    // Expression
    const ast::ExprLiteral* parse_expr_literal();
    const ast::ExprSymbol* parse_expr_symbol();
    const ast::Expr* parse_expr_group_or_tuple();
    const ast::ExprDynAccess* parse_expr_dyn_access(const ast::Expr* expr);
    const ast::ExprStaticAccess* parse_expr_st_access(const ast::Expr* expr);
    const ast::ExprUnary* parse_expr_unary(const ast::Expr* expr);
    const ast::ExprCall* parse_expr_call(const ast::Expr* expr);
    const ast::ExprSubscript* parse_expr_subscript(const ast::Expr* expr);
    const ast::ExprCast* parse_expr_cast(const ast::Expr* expr);
    const ast::ExprTernary* parse_expr_ternary(const ast::Expr* expr);
    const ast::ExprArray* parse_expr_array();
    const ast::ExprLambda* parse_expr_lambda();
    const ast::Expr* parse_expr_primary();
    const ast::Expr* parse_expr_affix();
    const ast::Expr* parse_expr(int minPrec = 0);

    // Types
    const ast::TypeBuiltin* parse_type_builtin();
    const ast::TypeArray* parse_type_array();
    const ast::TypeDict* parse_type_dict();
    const ast::TypeFunc* parse_type_function();
    const ast::Type* parse_type();

    // Statement
    const ast::StmtScope* parse_stmt_scope();
    const ast::StmtVarDecl* parse_stmt_var_decl(bool allowSemicolon);
    const ast::StmtFor* parse_stmt_for();
    const ast::StmtForEach* parse_stmt_for_each();
    const ast::StmtIf* parse_stmt_if();
    const ast::StmtWhile* parse_stmt_while();
    const ast::StmtAssign* parse_stmt_assign(const ast::Expr* expr);
    const ast::StmtReturn* parse_stmt_return();
    const ast::StmtEnum* parse_stmt_enum();
    const ast::StmtModule* parse_stmt_module();
    const ast::StmtImport* parse_stmt_import();
    const ast::StmtFunctionDecl* parse_stmt_func_decl();
    const ast::StmtStructDecl* parse_stmt_struct_decl();
    const ast::StmtTypeDecl* parse_stmt_type_decl();
    const ast::StmtUsing* parse_stmt_using_decl();
    const ast::Stmt* parse_stmt();

  private:
    DiagContext& m_diags;
    const std::string& m_source;
    const Token* const* m_cursor;
    ScopedAllocator m_alloc;
};

} // namespace via
