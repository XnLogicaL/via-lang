// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "constexpr.h"

namespace via {

namespace core {

namespace sema {

using namespace parser::ast;

// TODO: This function must perform deeper constexpr checks, such as constexpr
// variables and members.
bool is_constexpr(SemaContext& ctx, const ExprNode* expr) {
  if TRY_COERCE (const NodeExprLit, lit, expr)
    return true;
  else if TRY_COERCE (const NodeExprGroup, grp, expr)
    return is_constexpr(ctx, grp->expr);
  else if TRY_COERCE (const NodeExprUn, un, expr)
    return is_constexpr(ctx, un->expr);
  else if TRY_COERCE (const NodeExprBin, bin, expr)
    return is_constexpr(ctx, bin->lhs) && is_constexpr(ctx, bin->rhs);
  else if TRY_COERCE (const NodeExprTuple, tup, expr) {
    for (const auto* val : tup->vals)
      if (!is_constexpr(ctx, val))
        return false;

    return true;
  }

  return false;
}

}  // namespace sema

}  // namespace core

}  // namespace via
