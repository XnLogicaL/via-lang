// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "ast.h"

namespace via {

namespace detail {

inline usize DEFAULT_DEPTH = 0;

#define TAB                 String(depth * 4, ' ')
#define FORMAT(...)         TAB + std::format(__VA_ARGS__)
#define TRY_COERCE(T, a, b) (const T* a = dynamic_cast<const T*>(b))

static String tpb_to_string(const TupleBinding* tpb) {
  usize depth = 0;
  return FORMAT("TupleBinding[{}]", __vec_to_string(tpb->binds, [](const auto& e) {
                  return __ast_to_string_expr(e, DEFAULT_DEPTH);
                }));
}

static String lvalue_to_string(const LValue* lval) {
  if (lval->kind == LValue::LVK_SYM)
    return __ast_to_string_expr(lval->sym, DEFAULT_DEPTH);
  else if (lval->kind == LValue::LVK_TPB)
    return tpb_to_string(lval->tpb);

  std::unreachable();
}

String __ast_to_string_expr(const ExprNode* e, usize& depth) {
  if TRY_COERCE (NodeExprLit, lit, e)
    return FORMAT(
      "NodeExprLit({}, {})",
      magic_enum::enum_name(lit->tok->kind),
      String(lit->tok->lexeme, lit->tok->size)
    );
  else if TRY_COERCE (NodeExprSym, sym, e)
    return FORMAT("NodeExprSym({})", String(sym->tok->lexeme, sym->tok->size));
  else if TRY_COERCE (NodeExprUn, un, e)
    return FORMAT(
      "NodeExprUn({}, {})",
      String(un->op->lexeme, un->op->size),
      __ast_to_string_expr(un->expr, DEFAULT_DEPTH)
    );
  else if TRY_COERCE (NodeExprBin, bin, e)
    return FORMAT(
      "NodeExprBin({}, {}, {})",
      String(bin->op->lexeme, bin->op->size),
      __ast_to_string_expr(bin->lhs, DEFAULT_DEPTH),
      __ast_to_string_expr(bin->rhs, DEFAULT_DEPTH)
    );
  else if TRY_COERCE (NodeExprGroup, grp, e)
    return FORMAT("NodeExprGroup({})", __ast_to_string_expr(grp->expr, DEFAULT_DEPTH));
  else if TRY_COERCE (NodeExprCall, call, e)
    return FORMAT(
      "NodeExprCall({}, {})",
      __ast_to_string_expr(call->lval, DEFAULT_DEPTH),
      __vec_to_string<ExprNode*>(
        call->args, [](ExprNode* const& val) { return __ast_to_string_expr(val, DEFAULT_DEPTH); }
      )
    );
  else if TRY_COERCE (NodeExprTuple, tup, e)
    return FORMAT("NodeExprTuple({})", __vec_to_string(tup->vals, [](const auto& expr) {
                    return __ast_to_string_expr(expr, DEFAULT_DEPTH);
                  }));

  return FORMAT("<unknown-expr>");
}

String __ast_to_string_stmt(StmtNode* s, usize& depth) {
  if TRY_COERCE (NodeStmtScope, scope, s) {
    std::ostringstream oss;
    oss << FORMAT("NodeStmtScope()\n");
    depth++;

    for (const auto& stmt : scope->stmts)
      oss << __ast_to_string_stmt(stmt, depth) << "\n";

    depth--;
    oss << FORMAT("End()");
    return oss.str();
  }
  else if TRY_COERCE (NodeStmtVar, var, s) {
    String rvalue = __ast_to_string_expr(var->rval, DEFAULT_DEPTH);
    String lvalue = var->lval->kind == LValue::LVK_SYM
      ? __ast_to_string_expr(var->lval->sym, DEFAULT_DEPTH)
      : tpb_to_string(var->lval->tpb);

    return FORMAT("NodeStmtVar({}, {})", lvalue, rvalue);
  }
  else if TRY_COERCE (NodeStmtIf, ifs, s) {
    std::ostringstream oss;
    oss << "NodeStmtIf()\n";
    depth++;

    for (const auto& br : ifs->brs) {
      oss << FORMAT("Branch({})\n", __ast_to_string_expr(br.cnd, DEFAULT_DEPTH));
      depth++;

      for (StmtNode* const& stmt : br.br->stmts)
        oss << __ast_to_string_stmt(stmt, depth) << "\n";

      depth--;
      oss << FORMAT("End()\n");
    }

    depth--;
    oss << FORMAT("End()");
    return oss.str();
  }
  else if TRY_COERCE (NodeStmtFor, fors, s) {
    std::ostringstream oss;
    oss << FORMAT(
      "NodeStmtFor({}, {}, {})\n",
      __ast_to_string_stmt(fors->init, DEFAULT_DEPTH),
      __ast_to_string_expr(fors->target, DEFAULT_DEPTH),
      __ast_to_string_expr(fors->step, DEFAULT_DEPTH)
    );
    depth++;

    for (const auto& stmt : fors->br->stmts)
      oss << __ast_to_string_stmt(stmt, depth) << "\n";

    depth--;
    oss << FORMAT("End()");
    return oss.str();
  }
  else if TRY_COERCE (NodeStmtForEach, fors, s) {
    std::ostringstream oss;
    oss << FORMAT(
      "NodeStmtForEach({}, {})\n",
      lvalue_to_string(fors->lval),
      __ast_to_string_expr(fors->iter, DEFAULT_DEPTH)
    );
    depth++;

    for (const auto& stmt : fors->br->stmts)
      oss << __ast_to_string_stmt(stmt, depth) << "\n";

    depth--;
    oss << FORMAT("End()");
    return oss.str();
  }
  else if TRY_COERCE (NodeStmtWhile, whs, s) {
    std::ostringstream oss;
    oss << FORMAT("NodeStmtWhile({})\n", __ast_to_string_expr(whs->cnd, DEFAULT_DEPTH));
    depth++;

    for (StmtNode* const& stmt : whs->br->stmts)
      oss << __ast_to_string_stmt(stmt, depth) << "\n";

    depth--;
    oss << FORMAT("End()");
    return oss.str();
  }
  else if TRY_COERCE (NodeStmtEmpty, _, s)
    return FORMAT("NodeStmtEmpty()");

  return FORMAT("<unknown-stmt>");
}

String __ast_to_string_type(TypeNode* t, usize& depth) {
  return FORMAT("<unknown-type>");
}

} // namespace detail

void dump_stmt(StmtNode* stmt, usize& depth) {
  std::cout << detail::__ast_to_string_stmt(stmt, depth) << "\n";
}

} // namespace via
