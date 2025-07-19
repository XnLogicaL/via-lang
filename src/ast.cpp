// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ast.h"

namespace via {

namespace detail {

inline usize DEFAULT_DEPTH = 0;

#define TAB                 String(__depth, ' ')
#define FORMAT(...)         TAB + std::format(__VA_ARGS__)
#define TRY_COERCE(T, a, b) (const T* a = dynamic_cast<const T*>(b))

String __ast_to_string_expr(const ExprNode* __e, usize& __depth) {
  if TRY_COERCE (NodeExprLit, lit, __e)
    return FORMAT(
      "NodeExprLit({}, {})",
      magic_enum::enum_name(lit->tok->kind),
      String(lit->tok->lexeme, lit->tok->size)
    );
  else if TRY_COERCE (NodeExprSym, sym, __e)
    return FORMAT("NodeExprSym({})", String(sym->tok->lexeme, sym->tok->size));
  else if TRY_COERCE (NodeExprUn, un, __e)
    return FORMAT(
      "NodeExprUn({}, {})",
      String(un->op->lexeme, un->op->size),
      __ast_to_string_expr(un->expr, DEFAULT_DEPTH)
    );
  else if TRY_COERCE (NodeExprBin, bin, __e)
    return FORMAT(
      "NodeExprBin({}, {}, {})",
      String(bin->op->lexeme, bin->op->size),
      __ast_to_string_expr(bin->lhs, DEFAULT_DEPTH),
      __ast_to_string_expr(bin->rhs, DEFAULT_DEPTH)
    );
  else if TRY_COERCE (NodeExprGroup, grp, __e)
    return FORMAT("NodeExprGroup({})", __ast_to_string_expr(grp->expr, DEFAULT_DEPTH));
  else if TRY_COERCE (NodeExprCall, call, __e)
    return FORMAT(
      "NodeExprCall({}, {})",
      __ast_to_string_expr(call->lval, DEFAULT_DEPTH),
      __vec_to_string<ExprNode*>(
        call->args, [](ExprNode* const& val) { return __ast_to_string_expr(val, DEFAULT_DEPTH); }
      )
    );

  return FORMAT("<unknown-expr>");
}

String __ast_to_string_stmt(StmtNode* __s, usize& __depth) {
  // if TRY_COERCE (NodeStmtScope, scope, __s)
  //   return FORMAT("NodeStmtScope({})", __vec_to_string(scope->stmts, [](StmtNode* const& stmt) {
  //                   return __ast_to_string_stmt(stmt, DEFAULT_DEPTH);
  //                 }));
  if TRY_COERCE (NodeStmtIf, ifs, __s) {
    std::ostringstream oss;
    oss << "NodeStmtIf()\n";

    __depth++;

    for (const auto& br : ifs->brs) {
      oss << FORMAT("Branch({})\n", __ast_to_string_expr(br.cnd, DEFAULT_DEPTH));

      __depth++;

      for (StmtNode* const& stmt : br.br->stmts)
        oss << __ast_to_string_stmt(stmt, __depth) << "\n";

      __depth--;
      oss << FORMAT("End()\n");
    }

    __depth--;

    oss << FORMAT("End()");
    return oss.str();
  }
  else if TRY_COERCE (NodeStmtWhile, whs, __s) {
    std::ostringstream oss;
    oss << FORMAT("NodeStmtWhile({})\n", __ast_to_string_expr(whs->cnd, DEFAULT_DEPTH));

    __depth++;

    for (StmtNode* const& stmt : whs->br->stmts)
      oss << __ast_to_string_stmt(stmt, __depth) << "\n";

    __depth--;

    oss << FORMAT("End()");
    return oss.str();
  }

  return FORMAT("<unknown-stmt>");
}

String __ast_to_string_type(TypeNode* __t, usize& __depth) {
  return FORMAT("<unknown-type>");
}

} // namespace detail

void dump_stmt(StmtNode* stmt, usize& depth) {
  std::cout << detail::__ast_to_string_stmt(stmt, depth) << "\n";
}

} // namespace via
