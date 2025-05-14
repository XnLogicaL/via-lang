// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_AST_H
#define VIA_HAS_HEADER_AST_H

#include <lexloc.h>
#include <token.h>
#include <color.h>
#include <format-vector.h>
#include <tvalue.h>

namespace via {

enum class AstKind {
  EXPR_Lit,
  EXPR_Sym,
  EXPR_Un,
  EXPR_Bin,
  EXPR_Call,
  EXPR_Group,
  EXPR_Cast,
  EXPR_Idx,
  EXPR_Step,
  EXPR_Arr,
  EXPR_Intr,

  TYPE_Auto,
  TYPE_Prim,
  TYPE_Gen,
  TYPE_Uni,
  TYPE_Opt,
  TYPE_Fun,
  TYPE_Arr,
  TYPE_Dict,
  TYPE_Obj,

  STMT_Decl,
  STMT_Scope,
  STMT_Func,
  STMT_Asgn,
  STMT_If,
  STMT_Ret,
  STMT_Whi,
  STMT_Brk,
  STMT_Cont,
  STMT_Def,
  STMT_Expr,
};

struct AstNode;

struct Attribute {
  size_t argc;
  const char* ident;
  const char** args;
};

struct Parameter {
  const char* id;
  AstNode* type;
};

/**
 * Literal Expression Node
 * Represents a primitive literal value. Can be a Nil, Int, floating point, Bool or String
 * value.
 *
 * <literal> ::= <Int> | <float> | <String> | <Bool> | "Nil"
 * <Int> ::= ("0x"|"0b")?[0-9A-Fa-f]+
 * <float>   ::= [0-9]+.[0-9]+
 * <String>  ::= "\"" <characters> "\""
 * <Bool> ::= "true" | "false"
 */
struct NodeLitExpr {
  Value::Tag kind;
  union Un {
    bool b;
    int i;
    float f;
    const char* s;
  } u;
};

/**
 * Symbol Expression Node
 * Represents a symbol that could be a variable, index, property or global.
 *
 * <identifier>  ::= [A-Za-z_][A-Za-z0-9_]+
 * <symbol_expr> ::= <identifier>
 */
struct NodeSymExpr {
  const char* symbol;
};

/**
 * Unary Expression Node
 * Represents a unary expression node that applies an operator on a single expression.
 *
 * <operator>   ::= "+" | "-" | "*" | "/" | "++" | "--" | "#" | "^" | "%"
 * <unary_expr> ::= <operator> <expression>
 */
struct NodeUnExpr {
  TokenType op;
  AstNode* expr;
};

/**
 * Group Expression Node
 * Represents a grouping expression that houses a single expression. Mainly used on the parser level
 * to determine operator precedence.
 *
 * <group_expr> ::= "(" <expression> ")"
 */
struct NodeGroupExpr {
  AstNode* expr;
};

/**
 * Call Expression Node
 * Represents a function call expression with an argument list.
 *
 * <call_expr> ::= <expression> "(" <arg_list>? ")"
 * <arg_list>  ::= <expression> ("," <expression>)*
 */
struct NodeCallExpr {
  size_t argc;
  AstNode* callee;
  AstNode** args;
};

/**
 * Index Expression Node
 * Represents a subscript expression that holds a target and index expression.
 *
 * <index_expr>   ::= <primary_expr> <accessor>*
 * <accessor>     ::= "[" <expression> "]" | "." <identifier>
 * <primary_expr> ::= <identifier> | <literal> | "(" <expression> ")"
 */
struct NodeIndexExpr {
  AstNode* obj;
  AstNode* idx;
};

struct NodeBinExpr {
  TokenType op;
  AstNode* lhs;
  AstNode* rhs;
};

struct NodeCastExpr {
  AstNode* expr;
  AstNode* ty;
};

struct NodeStepExpr {
  TokenType op;
  AstNode* expr;
};

struct NodeArrExpr {
  size_t valc;
  AstNode** vals;
};

struct NodeIntrExpr {
  size_t exprc;
  const char* id;
  AstNode** exprs;
};

struct NodePrimType {
  const char* id;
  Value::Tag type;
};

struct NodeGenType {
  size_t genc;
  const char* id;
  AstNode** gens;
};

struct NodeOptType {
  AstNode* type;
};

struct NodeUnionType {
  AstNode* lhs;
  AstNode* rhs;
};

struct NodeFuncType {
  size_t paramc;
  AstNode* rets;
  Parameter* params;
};

struct NodeArrType {
  AstNode* type;
};

struct NodeDictType {
  AstNode* type;
};

struct NodeObjType {};

struct NodeDeclStmt {
  bool glb;
  const char* id;
  AstNode* rval;
  AstNode* type;
};

struct NodeScopeStmt {
  size_t stmtc;
  AstNode** stmts;
};

struct NodeFuncDeclStmt {
  bool glb;
  size_t paramc;
  const char* id;
  AstNode* body;
  AstNode* rets;
  struct Parameter* params;
};

struct NodeAsgnStmt {
  enum TokenType aug;
  AstNode* lval;
  AstNode* rval;
};

struct NodeIfStmt {
  struct Elif {
    LexLocation loc;
    AstNode* cond;
    AstNode* scp;
  };

  size_t elifc;
  AstNode* cond;
  AstNode* scp;
  AstNode* els;
  Elif* elifs;
};

struct NodeRetStmt {
  AstNode* expr;
};

struct NodeWhileStmt {
  AstNode* cond;
  AstNode* body;
};

struct NodeDeferStmt {
  AstNode* stmt;
};

struct NodeExprStmt {
  AstNode* expr;
};

struct AstNode {
  LexLocation loc;
  AstKind kind;

  union Un {
    NodeLitExpr e_lit;
    NodeSymExpr e_sym;
    NodeUnExpr e_un;
    NodeBinExpr e_bin;
    NodeCallExpr e_call;
    NodeGroupExpr e_grp;
    NodeCastExpr e_cast;
    NodeIndexExpr e_idx;
    NodeStepExpr e_step;
    NodeArrExpr e_arr;
    NodeIntrExpr e_intr;

    NodePrimType t_prim;
    NodeGenType t_gen;
    NodeUnionType t_un;
    NodeOptType t_opt;
    NodeFuncType t_fun;
    NodeArrType t_arr;
    NodeDictType t_dict;
    NodeObjType t_obj;

    NodeDeclStmt s_decl;
    NodeScopeStmt s_scope;
    NodeFuncDeclStmt s_func;
    NodeAsgnStmt s_asgn;
    NodeIfStmt s_if;
    NodeRetStmt s_ret;
    NodeWhileStmt s_whi;
    NodeDeferStmt s_def;
    NodeExprStmt s_expr;
  } u;
};

} // namespace via

#endif
