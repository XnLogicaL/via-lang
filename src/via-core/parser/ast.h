// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_AST_H_
#define VIA_CORE_AST_H_

#include <via/config.h>
#include "lexer/location.h"
#include "lexer/token.h"

#define TRY_COERCE(T, a, b) (const T* a = dynamic_cast<const T*>(b))

namespace via {

namespace core {

namespace parser {

namespace ast {

using lex::AbsLocation;
using lex::Token;

struct ExprNode {
  AbsLocation loc;
  virtual String get_dump(usize& depth) const = 0;
};

struct StmtNode {
  AbsLocation loc;
  virtual String get_dump(usize& depth) const = 0;
};

struct TypeNode {
  AbsLocation loc;
  virtual String get_dump(usize& depth) const = 0;
};

struct NodeExprSym;

struct TupleBinding {
  Vec<NodeExprSym*> binds;
  AbsLocation loc;
};

struct LValue {
  enum {
    Symbol,
    Tpb,
  } kind;

  union {
    TupleBinding* tpb;
    NodeExprSym* sym;
  };
};

struct Parameter {
  NodeExprSym* sym;
  TypeNode* type;
  AbsLocation loc;
};

#define COMMON_HEADER(klass) \
  using klass::loc;          \
  String get_dump(usize& depth) const override;

struct NodeExprLit : public ExprNode {
  COMMON_HEADER(ExprNode)
  Token* tok;
};

struct NodeExprSym : public ExprNode {
  COMMON_HEADER(ExprNode)
  Token* tok;
};

struct NodeExprUn : public ExprNode {
  COMMON_HEADER(ExprNode)
  Token* op;
  ExprNode* expr;
};

struct NodeExprBin : public ExprNode {
  COMMON_HEADER(ExprNode)
  Token* op;
  ExprNode *lhs, *rhs;
};

struct NodeExprGroup : public ExprNode {
  COMMON_HEADER(ExprNode)
  ExprNode* expr;
};

struct NodeExprCall : public ExprNode {
  COMMON_HEADER(ExprNode)
  ExprNode* lval;
  Vec<ExprNode*> args;
};

struct NodeExprSubs : public ExprNode {
  COMMON_HEADER(ExprNode)
  ExprNode *lval, *idx;
};

struct NodeExprTuple : public ExprNode {
  COMMON_HEADER(ExprNode)
  Vec<ExprNode*> vals;
};

struct NodeStmtScope;

struct NodeExprLambda : public ExprNode {
  COMMON_HEADER(ExprNode)
  Vec<Parameter> pms;
  NodeStmtScope* scope;
};

struct NodeStmtVar : public StmtNode {
  COMMON_HEADER(StmtNode)
  LValue* lval;
  ExprNode* rval;
};

struct NodeStmtScope : public StmtNode {
  COMMON_HEADER(StmtNode)
  Vec<StmtNode*> stmts;
};

struct NodeStmtIf : public StmtNode {
  struct Branch {
    ExprNode* cnd;
    NodeStmtScope* br;
  };

  COMMON_HEADER(StmtNode)
  Vec<Branch> brs;
};

struct NodeStmtFor : public StmtNode {
  COMMON_HEADER(StmtNode)
  NodeStmtVar* init;
  ExprNode* target;
  ExprNode* step;
  NodeStmtScope* br;
};

struct NodeStmtForEach : public StmtNode {
  COMMON_HEADER(StmtNode)
  LValue* lval;
  ExprNode* iter;
  NodeStmtScope* br;
};

struct NodeStmtWhile : public StmtNode {
  COMMON_HEADER(StmtNode)
  ExprNode* cnd;
  NodeStmtScope* br;
};

struct NodeStmtAssign : public StmtNode {
  COMMON_HEADER(StmtNode)
  ExprNode *lval, *rval;
};

struct NodeStmtEmpty : public StmtNode {
  COMMON_HEADER(StmtNode)
};

struct NodeStmtExpr : public StmtNode {
  COMMON_HEADER(StmtNode)
  ExprNode* expr;
};

void dump_stmt(StmtNode* stmt, usize& depth);

#undef COMMON_HEADER

}  // namespace ast

}  // namespace parser

}  // namespace core

}  // namespace via

#endif
