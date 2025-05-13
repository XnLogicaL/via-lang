// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_HAS_HEADER_AST_H
#define VIA_HAS_HEADER_AST_H

#include <lex/lexloc.h>
#include <lex/token.h>
#include <utility/color.h>
#include <utility/format-vector.h>
#include <interpreter/tvalue.h>

namespace via {

struct Node;

struct Attribute {
  size_t argc;
  const char* ident;
  const char** args;
};

struct Parameter {
  const char* id;
  struct Node* type;
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
  using variant = std::variant<std::monostate, int, float, bool, std::string>;
  variant value;
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
  enum TokenType op;
  struct Node* expr;
};

/**
 * Group Expression Node
 * Represents a grouping expression that houses a single expression. Mainly used on the parser level
 * to determine operator precedence.
 *
 * <group_expr> ::= "(" <expression> ")"
 */
struct NodeGroupExpr {
  struct Node* expr;
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
  struct Node* callee;
  struct Node** args;
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
  struct Node* obj;
  struct Node* idx;
};

struct NodeBinExpr {
  enum TokenType op;
  struct Node* lhs;
  struct Node* rhs;
};

struct NodeCastExpr {
  struct Node* expr;
  struct Node* ty;
};

struct StepExprNode {
  enum TokenType op;
  struct Node* expr;
};

struct NodeArrExpr {
  size_t valc;
  struct Node** vals;
};

struct NodeIntrExpr {
  size_t exprc;
  const char* id;
  struct Node** exprs;
};

struct NodePrimType {
  const char* id;
  Value::Tag type;
};

struct NodeGenType {
  size_t genc;
  const char* id;
  struct Node** generics;
};

struct NodeOptType {
  struct Node* type;
};

struct NodeUnionType {
  struct Node* lhs;
  struct Node* rhs;
};

struct NodeFuncType {
  size_t paramc;
  struct Node* rets;
  struct Node** params;
};

struct NodeArrType {
  struct Node* type;
};

struct NodeDictType {
  struct Node* type;
};

struct NodeObjType {};

struct NodeDeclStmt {
  bool glb;
  struct Token id;
  struct Node* rval;
  struct Node* type;
};

struct NodeScopeStmt {
  size_t stmtc;
  struct Node** stmts;
};

struct NodeFuncDeclStmt {
  bool glb;
  size_t paramc;
  const char* id;
  struct Node* body;
  struct Node* rets;
  struct Parameter* params;
};

struct NodeAsgnStmt {
  enum TokenType aug;
  struct Node* lval;
  struct Node* rval;
};

struct IfStmtNode {
  struct Elif {
    size_t len;
    struct LexLocation loc;
    struct Node* cond;
    struct Node* scp;
  };

  size_t elifc;
  struct Node* cond;
  struct Node* scp;
  struct Node* els;
  struct Elif* elifs;
};

struct ReturnStmtNode {
  struct Node* expr;
};

struct WhileStmtNode {
  struct Node* cond;
  struct Node* body;
};

struct DeferStmtNode {
  struct Node* stmt;
};

struct ExprStmtNode {
  struct Node* expression;
};

struct Node {
  enum class Type {
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
    STMT_Brk,
    STMT_Cont,
    STMT_Def,
    STMT_Expr,
  } type;

  union Un {
    NodeLitExpr e_lit;
    NodeSymExpr e_sym;
    NodeUnExpr e_un;
    NodeBinExpr e_bin;
    NodeCallExpr e_call;
    NodeGroupExpr e_grp;
    NodeCastExpr e_cast;
    NodeIndexExpr e_idx;
    NodeFuncDeclStmt e_step;
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
    IfStmtNode s_if;
    ReturnStmtNode s_ret;
    DeferStmtNode s_def;
    ExprStmtNode s_expr;
  } u;

  size_t len;
  LexLocation loc;
};

} // namespace via

#endif
