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

  // clang-format off
  // Special
  const ast::AccessIdent*    parseAccessIdent();
  const ast::Path*           parseStaticPath();
  const ast::Expr*           parseLValue();
  const ast::Parameter*      parseParameter();
  const ast::AttributeGroup* parseAttribGroup();

  // Expression
  const ast::ExprLit*          parseExprLit();
  const ast::ExprSymbol*       parseExprSymbol();
  const ast::Expr*             parseExprGroupOrTuple();
  const ast::ExprDynAccess*    parseExprDynAccess(const ast::Expr* expr);
  const ast::ExprStaticAccess* parseExprStAccess(const ast::Expr* expr);
  const ast::ExprUnary*        parseExprUnary(const ast::Expr* expr);
  const ast::ExprCall*         parseExprCall(const ast::Expr* expr);
  const ast::ExprSubscript*    parseExprSubscript(const ast::Expr* expr);
  const ast::ExprCast*         parseExprCast(const ast::Expr* expr);
  const ast::ExprTernary*      parseExprTernary(const ast::Expr* expr);
  const ast::ExprArray*        parseExprArray();
  const ast::ExprLambda*       parseExprLambda();
  const ast::Expr*             parseExprPrimary();
  const ast::Expr*             parseExprAffix();
  const ast::Expr*             parseExpr(int minPrec = 0);

  // Types
  const ast::TypeBuiltin* parseTypeBuiltin();
  const ast::TypeArray*   parseTypeArray();
  const ast::TypeDict*    parseTypeDict();
  const ast::TypeFunc*    parseTypeFunc();
  const ast::Type*        parseType();

  // Statement
  const ast::StmtScope*        parseStmtScope();
  const ast::StmtVarDecl*      parseStmtVarDecl(bool allowSemicolon);
  const ast::StmtFor*          parseStmtFor();
  const ast::StmtForEach*      parseStmtForEach();
  const ast::StmtIf*           parseStmtIf();
  const ast::StmtWhile*        parseStmtWhile();
  const ast::StmtAssign*       parseStmtAssign(const ast::Expr* expr);
  const ast::StmtReturn*       parseStmtReturn();
  const ast::StmtEnum*         parseStmtEnum();
  const ast::StmtModule*       parseStmtModule();
  const ast::StmtImport*       parseStmtImport();
  const ast::StmtFunctionDecl* parseStmtFuncDecl();
  const ast::StmtStructDecl*   parseStmtStructDecl();
  const ast::StmtTypeDecl*     parseStmtTypeDecl();
  const ast::StmtUsing*        parseStmtUsingDecl();
  const ast::Stmt*             parseStmt();
  // clang-format on

 private:
  DiagContext& mDiag;
  const std::string& mSource;
  const Token* const* mCursor;
  Allocator mAlloc;
};

}  // namespace via
