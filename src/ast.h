// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_AST_H
#define VIA_AST_H

#include "common.h"
#include "lextoken.h"
#include <arena/arena.h>

namespace via {

enum AstKind {
  AK_EXPR_LIT,
  AK_EXPR_SYM,
  AK_EXPR_UN,
  AK_EXPR_BIN,
  AK_EXPR_GROUP,
  AK_EXPR_CALL,
  AK_EXPR_SUBS,

  AK_STMT_DECL,
  AK_STMT_FUNC,
};

struct AstNode;

struct NodeExprLit {
  Token* token;
};

struct NodeExprSym {
  Token* token;
};

struct NodeExprUn {
  Token* op;
  AstNode* expr;
};

struct NodeExprBin {
  Token* op;
  AstNode *lhs, *rhs;
};

struct AstNode {
  AstKind kind;
  union {
    NodeExprLit expr_lit;
    NodeExprSym expr_sym;
  } u;
};

Location get_ast_location(AstNode* node);

} // namespace via

#endif
