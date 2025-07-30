// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "constexpr.h"

namespace via {

namespace core {

namespace sema {

using lex::TokenKind;
using namespace parser::ast;

// TODO: This function must perform deeper constexpr checks, such as constexpr
// variables and members.
bool is_constexpr(const ExprNode* expr) {
  if TRY_COERCE (NodeExprLit, lit, expr)
    return true;
  else if TRY_COERCE (NodeExprGroup, grp, expr)
    return is_constexpr(grp->expr);
  else if TRY_COERCE (NodeExprUn, un, expr)
    return is_constexpr(un->expr);
  else if TRY_COERCE (NodeExprBin, bin, expr)
    return is_constexpr(bin->lhs) && is_constexpr(bin->rhs);
  else if TRY_COERCE (NodeExprTuple, tup, expr) {
    for (const auto* val : tup->vals)
      if (!is_constexpr(val))
        return false;
    return true;
  }

  return false;
}

}  // namespace sema

}  // namespace core

}  // namespace via
