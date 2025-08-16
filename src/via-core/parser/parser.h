// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_PARSER_H_
#define VIA_CORE_PARSER_H_

#include <mimalloc.h>
#include <via/config.h>
#include <via/types.h>
#include "ast.h"
#include "constexpr_stof.h"
#include "constexpr_stoi.h"
#include "diagnostics.h"
#include "lexer/lexer.h"
#include "lexer/token.h"
#include "memory.h"

namespace via {

using AstBuf = Vec<ast::StmtNode*>;

class Parser final {
 public:
  Parser(const FileBuf& source, const TokenBuf& tokens, Diagnostics& diag)
      : source(source), cursor(tokens.data), diag(diag) {}

  AstBuf parse();

 private:
  bool match(Token::Kind kind, int ahead = 0);
  bool optional(Token::Kind kind);

  Token* peek(int ahead = 0);
  Token* advance();
  Token* expect(Token::Kind kind, const char* task);

  // Special
  ast::TupleBinding* parse_tuple_binding();
  ast::LValue* parse_lvalue();

  // Expression
  ast::ExprNode* parse_primary();
  ast::ExprNode* parse_unary_or_postfix();
  ast::ExprNode* parse_expr(int min_prec = 0);

  // Statement
  ast::NodeStmtScope* parse_scope();
  ast::NodeStmtVar* parse_var();
  ast::NodeStmtFor* parse_for();
  ast::NodeStmtForEach* parse_foreach();
  ast::NodeStmtIf* parse_if();
  ast::NodeStmtWhile* parse_while();
  ast::StmtNode* parse_stmt();

 private:
  const FileBuf& source;
  Token** cursor;
  HeapAllocator alloc;
  Diagnostics& diag;
};

void dump_ast(const AstBuf& B);

}  // namespace via

#endif
