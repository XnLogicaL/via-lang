// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_PARSER_H_
#define VIA_CORE_PARSER_H_

#include <mimalloc.h>
#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
#include "diagnostics.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "memory.h"

namespace via
{

class Parser final
{
 public:
  Parser(const String& source, const TokenTree& tokens, DiagContext& diag)
      : m_source(source), m_cursor(tokens.cbegin().base()), m_diag(diag)
  {}

 public:
  Allocator& get_allocator() { return m_alloc; }

  SyntaxTree parse();

 private:
  bool match(Token::Kind kind, int ahead = 0);
  bool optional(Token::Kind kind);

  Token* peek(int ahead = 0);
  Token* advance();
  Token* expect(Token::Kind kind, const char* task);

  // Special
  ast::TupleBinding* parse_tuple_binding();
  ast::Path* parse_static_path();
  ast::Path* parse_dynamic_path();
  ast::LValue* parse_lvalue();
  ast::PlValue* parse_plvalue();
  ast::Parameter* parse_parameter();
  ast::AttributeGroup* parse_attribute_group();

  // Expression
  ast::Expr* parse_expr_primary();
  ast::Expr* parse_expr_unary_or_postfix();
  ast::Expr* parse_expr(int min_prec = 0);

  // Types
  ast::TypeBuiltin* parse_type_builtin();
  ast::TypeArray* parse_type_array();
  ast::TypeDict* parse_type_dict();
  ast::TypeFunc* parse_type_func();
  ast::Type* parse_type();

  // Statement
  ast::StmtScope* parse_stmt_scope();
  ast::StmtVarDecl* parse_stmt_var(bool semicolon);
  ast::StmtFor* parse_stmt_for();
  ast::StmtForEach* parse_stmt_foreach();
  ast::StmtIf* parse_stmt_if();
  ast::StmtWhile* parse_stmt_while();
  ast::StmtAssign* parse_stmt_assign(ast::Expr* lhs);
  ast::StmtReturn* parse_stmt_return();
  ast::StmtEnum* parse_stmt_enum();
  ast::StmtModule* parse_stmt_module();
  ast::StmtImport* parse_stmt_import();
  ast::StmtFunctionDecl* parse_stmt_func();
  ast::StmtStructDecl* parse_stmt_struct();
  ast::StmtTypeDecl* parse_stmt_type();
  ast::StmtUsing* parse_stmt_using();
  ast::Stmt* parse_stmt();

 private:
  DiagContext& m_diag;
  const String& m_source;
  Token* const* m_cursor;
  Allocator m_alloc;
};

}  // namespace via

#endif
