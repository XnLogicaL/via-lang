// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_AST_H
#define VIA_AST_H

#include "common.h"
#include "lextoken.h"

namespace via {

struct ExprNode {
  virtual ~ExprNode() = 0;
};

struct StmtNode {
  virtual ~StmtNode() = 0;
};

struct TypeNode {
  virtual ~TypeNode() = 0;
};

struct NodeExprSym;

struct NodeExprLit : public ExprNode {
  Token* tok;
  Location loc;
};

struct NodeExprSym : public ExprNode {
  Token* tok;
  Location loc;
};

struct NodeExprUn : public ExprNode {
  Token* op;
  ExprNode* expr;
  Location loc;
};

struct NodeExprBin : public ExprNode {
  Token* op;
  ExprNode *lhs, *rhs;
  Location loc;
};

struct NodeExprGroup : public ExprNode {
  ExprNode* expr;
  Location loc;
};

struct NodeExprCall : public ExprNode {
  ExprNode* lval;
  Vec<ExprNode*> args;
  Location loc;
};

struct NodeExprSubs : public ExprNode {
  ExprNode *lval, *idx;
  Location loc;
};

struct NodeExprTuple : public ExprNode {
  Vec<ExprNode*> vals;
  Location loc;
};

struct Parameter {
  NodeExprSym* sym;
  TypeNode* type;
  Location loc;
};

struct NodeStmtScope;

struct NodeExprLambda : public ExprNode {
  Vec<Parameter> pms;
  NodeStmtScope* scope;
  Location loc;
};

struct NodeExprVar : public ExprNode {
  NodeExprSym* sym;
  ExprNode* val;
  Location loc;
};

struct NodeStmtScope : public StmtNode {
  Vec<StmtNode*> stmts;
  Location loc;
};

struct NodeStmtIf : public StmtNode {
  struct Branch {
    ExprNode* cnd;
    NodeStmtScope* br;
  };

  Vec<Branch> brs;
  Location loc;
};

struct TupleBinding {
  Vec<NodeExprSym*> binds;
  Location loc;
};

struct NodeStmtFor : public StmtNode {
  NodeExprVar* idx;
  ExprNode* target;
  ExprNode* step;
  Location loc;
};

struct NodeStmtForEach : public StmtNode {
  TupleBinding bind;
  ExprNode* iter;
  Location loc;
};

struct NodeStmtWhile : public StmtNode {
  ExprNode* cnd;
  NodeStmtScope* br;
  Location loc;
};

struct NodeStmtAssign : public StmtNode {
  ExprNode* lval;
  ExprNode* rval;
  Location loc;
};

struct NodeStmtExpr : public StmtNode {
  ExprNode* expr;
  Location loc;
};

} // namespace via

#endif
