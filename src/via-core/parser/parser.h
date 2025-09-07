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
  Parser(const std::string& source, const TokenTree& tokens, DiagContext& diag)
      : mSource(source), mCursor(tokens.cbegin().base()), mDiag(diag)
  {}

 public:
  Allocator& getAllocator() { return mAlloc; }
  SyntaxTree parse();

 private:
  bool match(Token::Kind kind, int ahead = 0);
  bool optional(Token::Kind kind);

  const Token* peek(int ahead = 0);
  const Token* advance();
  const Token* expect(Token::Kind kind, const char* task);

  // Special
  ast::AccessIdent* parseAccessIdent();
  ast::Path* parseStaticPath();
  ast::Expr* parseLValue();
  ast::Parameter* parseParameter();
  ast::AttributeGroup* parseAttribGroup();

  // Expression
  ast::Expr* parseExprPrimary();
  ast::Expr* parseExprAffix();
  ast::Expr* parseExpr(int minPrec = 0);

  // Types
  ast::TypeBuiltin* parseTypeBuiltin();
  ast::TypeArray* parseTypeArray();
  ast::TypeDict* parseTypeDict();
  ast::TypeFunc* parseTypeFunc();
  ast::Type* parseType();

  // Statement
  ast::StmtScope* parseStmtScope();
  ast::StmtVarDecl* parseStmtVarDecl(bool allowSemicolon);
  ast::StmtFor* parseStmtFor();
  ast::StmtForEach* parseStmtForEach();
  ast::StmtIf* parseStmtIf();
  ast::StmtWhile* parseStmtWhile();
  ast::StmtAssign* parseStmtAssign(ast::Expr* lhs);
  ast::StmtReturn* parseStmtReturn();
  ast::StmtEnum* parseStmtEnum();
  ast::StmtModule* parseStmtModule();
  ast::StmtImport* parseStmtImport();
  ast::StmtFunctionDecl* parseStmtFuncDecl();
  ast::StmtStructDecl* parseStmtStructDecl();
  ast::StmtTypeDecl* parseStmtTypeDecl();
  ast::StmtUsing* parseStmtUsingDecl();
  ast::Stmt* parseStmt();

 private:
  DiagContext& mDiag;
  const std::string& mSource;
  const Token* const* mCursor;
  Allocator mAlloc;
};

}  // namespace via

#endif
