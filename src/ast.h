// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_AST_H
#define VIA_AST_H

#include "common.h"
#include "lextoken.h"

namespace via {

struct ExprNode {
  AbsLocation loc;
};

struct StmtNode {
  AbsLocation loc;
};

struct TypeNode {
  AbsLocation loc;
};

struct NodeExprSym;

struct NodeExprLit : public ExprNode {
  using ExprNode::loc;
  Token* tok;
};

struct NodeExprSym : public ExprNode {
  using ExprNode::loc;
  Token* tok;
};

struct NodeExprUn : public ExprNode {
  using ExprNode::loc;
  Token* op;
  ExprNode* expr;
};

struct NodeExprBin : public ExprNode {
  using ExprNode::loc;
  Token* op;
  ExprNode *lhs, *rhs;
};

struct NodeExprGroup : public ExprNode {
  using ExprNode::loc;
  ExprNode* expr;
};

struct NodeExprCall : public ExprNode {
  using ExprNode::loc;
  ExprNode* lval;
  Vec<ExprNode*> args;
};

struct NodeExprSubs : public ExprNode {
  using ExprNode::loc;
  ExprNode *lval, *idx;
};

struct NodeExprTuple : public ExprNode {
  using ExprNode::loc;
  Vec<ExprNode*> vals;
};

struct Parameter {
  NodeExprSym* sym;
  TypeNode* type;
  AbsLocation loc;
};

struct NodeStmtScope;

struct NodeExprLambda : public ExprNode {
  using ExprNode::loc;
  Vec<Parameter> pms;
  NodeStmtScope* scope;
};

struct NodeExprVar : public ExprNode {
  using ExprNode::loc;
  NodeExprSym* sym;
  ExprNode* val;
};

struct NodeStmtScope : public StmtNode {
  using StmtNode::loc;
  Vec<StmtNode*> stmts;
};

struct NodeStmtIf : public StmtNode {
  struct Branch {
    ExprNode* cnd;
    NodeStmtScope* br;
  };

  using StmtNode::loc;
  Vec<Branch> brs;
};

struct TupleBinding {
  Vec<NodeExprSym*> binds;
  AbsLocation loc;
};

struct NodeStmtFor : public StmtNode {
  using StmtNode::loc;
  NodeExprVar* idx;
  ExprNode* target;
  ExprNode* step;
};

struct NodeStmtForEach : public StmtNode {
  using StmtNode::loc;
  TupleBinding bind;
  ExprNode* iter;
};

struct NodeStmtWhile : public StmtNode {
  using StmtNode::loc;
  ExprNode* cnd;
  NodeStmtScope* br;
};

struct NodeStmtAssign : public StmtNode {
  using StmtNode::loc;
  ExprNode* lval;
  ExprNode* rval;
};

struct NodeStmtEmpty : public StmtNode {
  using StmtNode::loc;
};

struct NodeStmtExpr : public StmtNode {
  using StmtNode::loc;
  ExprNode* expr;
};

} // namespace via

#endif
