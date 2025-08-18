// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "constexpr.h"
#include "type.h"

namespace via {

namespace sema {

using namespace ast;
using namespace types;
using enum ConstValue::Kind;

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

using EvalResult = Result<ConstValue, via::String>;

static UnOp to_unop(Token::Kind kind) {
  switch (kind) {
    case Token::Kind::MINUS:
      return UnOp::Neg;
    case Token::Kind::KW_NOT:
      return UnOp::Not;
    case Token::Kind::TILDE:
      return UnOp::Bnot;
    default:
      break;
  }

  VIA_BUG();
}

template <UnOp Op, type T>
static EvalResult evaluate_unary(Context& ctx, ConstValue&& cv) {
  using Result = unary_result_t<Op, T>;

  if constexpr (!is_valid_type_v<Result>)
    return std::unexpected(
        fmt::format("Invalid unary operation ({}) on type '{}'",
                    magic_enum::enum_name(Op), get_typename<T>(ctx)));

  switch (Op) {
    case UnOp::Neg:
      if constexpr (std::is_same_v<T, int_type>)
        return ConstValue(-cv.value<Int>());
      return ConstValue(-cv.value<Float>());
    case UnOp::Not:
      if constexpr (std::is_same_v<T, nil_type>)
        return ConstValue(true);
      else if constexpr (std::is_same_v<T, bool_type>)
        return ConstValue(!cv.value<Boolean>());

      return ConstValue(false);
    case UnOp::Bnot:
      return ConstValue(~cv.value<Int>());
  }

  VIA_BUG();
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

  VIA_BUG();
}

EvalResult to_constexpr(Context& ctx, const ExprNode* expr) {
  if TRY_COERCE (const NodeExprLit, lit, expr)
    return *ConstValue::from_literal_token(*lit->tok);
  else if TRY_COERCE (const NodeExprGroup, grp, expr)
    return to_constexpr(ctx, grp->expr);
  else if TRY_COERCE (const NodeExprUn, un, expr) {
    return to_constexpr(ctx, un->expr).and_then([&ctx, &un](ConstValue cv) {
      return std::visit(
          [&ctx, &un, &cv](types::type auto&& t) -> EvalResult {
            return dispatch_unary<std::decay_t<decltype(t)>>(
                ctx, to_unop(un->op->kind), std::move(cv));
          },
          get_type(ctx, un->expr));
    });
  }

  VIA_UNIMPLEMENTED("unimplemented to_constexpr case");
}

}  // namespace sema

}  // namespace via
