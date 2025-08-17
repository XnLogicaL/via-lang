// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "constexpr.h"

namespace via {

namespace sema {

using namespace ast;

// TODO: This function must perform deeper constexpr checks, such as constexpr
// variables and members.
bool is_constexpr(Context& ctx, const ExprNode* expr) {
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

ConstValue to_constexpr(Context& ctx, const ExprNode* expr) {
  return ConstValue(ConstValue::int_type(69));
}

}  // namespace sema

}  // namespace via
