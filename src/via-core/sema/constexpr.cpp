// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "constexpr.h"
#include "debug.h"
#include "type.h"

namespace via {

namespace sema {

using namespace ast;
using namespace types;
using enum ConstValue::Kind;

bool is_constexpr(Context& ctx, const ExprNode* expr) {
  if TRY_COERCE (const NodeExprLit, lit, expr) {
    return true;
  } else if TRY_COERCE (const NodeExprSym, sym, expr) {
    Frame& frame = ctx.stack.top();
    StringView symbol = sym->tok->to_string_view();

    if (auto lref = frame.get_local(symbol)) {
      return is_constexpr(ctx, lref->local.get_rval());
    }

    bug("is_constexpr symbol lookup failed");
  } else if TRY_COERCE (const NodeExprGroup, grp, expr) {
    return is_constexpr(ctx, grp->expr);
  } else if TRY_COERCE (const NodeExprUn, un, expr) {
    return is_constexpr(ctx, un->expr);
  } else if TRY_COERCE (const NodeExprBin, bin, expr) {
    return is_constexpr(ctx, bin->lhs) && is_constexpr(ctx, bin->rhs);
  } else if TRY_COERCE (const NodeExprTuple, tup, expr) {
    for (const auto* val : tup->vals)
      if (!is_constexpr(ctx, val))
        return false;

    return true;
  }

  return false;
}

using EvalResult = Result<ConstValue, via::String>;

template <UnOp Op, type T>
static EvalResult evaluate_unary(Context& ctx, ConstValue&& cv) {
  if constexpr (!is_valid_type_v<unary_result_t<Op, T>>)
    return std::unexpected(
        fmt::format("Invalid unary operation ({}) on type '{}'",
                    magic_enum::enum_name(Op), type_info<T>::name));

  switch (Op) {
    case UnOp::Neg:
      if constexpr (std::is_same_v<T, int_type<>>)
        return ConstValue(-cv.value<Int>());
      return ConstValue(-cv.value<Float>());
    case UnOp::Not:
      if constexpr (std::is_same_v<T, nil_type<>>)
        return ConstValue(true);
      else if constexpr (std::is_same_v<T, bool_type<>>)
        return ConstValue(!cv.value<Boolean>());

      return ConstValue(false);
    case UnOp::Bnot:
      return ConstValue(~cv.value<Int>());
  }

  bug("failed to evaluate unary constexpr");
}

template <type T>
static EvalResult dispatch_unary(Context& ctx, UnOp op, ConstValue&& cv) {
  switch (op) {
    case UnOp::Neg:
      return evaluate_unary<UnOp::Neg, T>(ctx, std::move(cv));
    case UnOp::Not:
      return evaluate_unary<UnOp::Not, T>(ctx, std::move(cv));
    case UnOp::Bnot:
      return evaluate_unary<UnOp::Bnot, T>(ctx, std::move(cv));
    default:
      break;
  }

  bug("failed to dispatch unary constexpr");
}

EvalResult to_constexpr(Context& ctx, const ExprNode* expr) {
  if TRY_COERCE (const NodeExprLit, lit, expr) {
    return *ConstValue::from_literal_token(*lit->tok);
  } else if TRY_COERCE (const NodeExprSym, sym, expr) {
    Frame& frame = ctx.stack.top();
    StringView symbol = sym->tok->to_string_view();

    if (auto lref = frame.get_local(symbol)) {
      return to_constexpr(ctx, lref->local.get_rval());
    }

    bug("attempt to fold non-constexpr symbol");
  } else if TRY_COERCE (const NodeExprGroup, grp, expr) {
    return to_constexpr(ctx, grp->expr);
  } else if TRY_COERCE (const NodeExprUn, un, expr) {
    return to_constexpr(ctx, un->expr)
        .and_then([&ctx, &un](ConstValue&& cv) -> EvalResult {
          if (auto ty = infer_type<type_list<>>(ctx, un->expr)) {
            return std::visit(
                [&](auto&& t) {
                  return dispatch_unary<std::decay_t<decltype(t)>>(
                      ctx, to_unop(un->op->kind), std::move(cv));
                },
                *ty);
          }

          bug("failed to infer inner type of unary constexpr");
        });
  }

  unimplemented("to_constexpr");
}

}  // namespace sema

}  // namespace via
