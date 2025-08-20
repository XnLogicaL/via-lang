// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_SEMA_TYPE_H_
#define VIA_CORE_SEMA_TYPE_H_

#include <via/config.h>
#include <via/types.h>
#include "ast/ast.h"
#include "context.h"
#include "type_base.h"
#include "type_operations.h"
#include "type_primitives.h"

namespace via {

namespace sema {

namespace types {

template <typelist Ts>
using any = Variant<nil_type<Ts>,
                    bool_type<Ts>,
                    int_type<Ts>,
                    float_type<Ts>,
                    string_type<Ts>,
                    array_type<Ts>,
                    dict_type<Ts>>;

};  // namespace types

template <types::typelist Ts>
Optional<types::any<Ts>> infer_type(Context& ctx, const ast::ExprNode* expr) {
  using types::UnOp;

  if TRY_COERCE (const ast::NodeExprLit, lit, expr) {
    switch (lit->tok->kind) {
      case Token::Kind::NIL:
        return types::nil_type{};
      case Token::Kind::TRUE:
      case Token::Kind::FALSE:
        return types::bool_type{};
      case Token::Kind::INT:
      case Token::Kind::XINT:
      case Token::Kind::BINT:
        return types::int_type{};
      case Token::Kind::FP:
        return types::float_type{};
      case Token::Kind::STRING:
        return types::string_type{};
      default:
        break;
    }

    bug("infer_type: bad literal");
  } else if TRY_COERCE (const ast::NodeExprSym, sym, expr) {
    Frame& frame = ctx.stack.top();
    StringView symbol = sym->tok->to_string_view();

    // TODO: TEMPORARY IMPLEMENTATION
    if (auto lref = frame.get_local(symbol))
      return infer_type<Ts>(ctx, lref->local.get_rval());

    bug("infer_type: bad symbol (lookup failure)");
  } else if TRY_COERCE (const ast::NodeExprUn, un, expr) {
    UnOp op = types::to_unop(un->op->kind);

    return infer_type<Ts>(ctx, un->expr)
        .and_then([&op](auto&& ty) -> types::any<Ts> {
          return std::visit(
              [op](auto&& t) -> types::any<Ts> {
                using T = std::decay_t<decltype(t)>;

                switch (op) {
                  case UnOp::Neg:
                    return types::invalid_or_t<
                        types::unary_result_t<UnOp::Neg, T>,
                        types::invalid_type<>>{};
                  case UnOp::Not:
                    return types::unary_result_t<UnOp::Not, T>{};
                  case UnOp::Bnot:
                    return types::invalid_or_t<
                        types::unary_result_t<UnOp::Bnot, T>,
                        types::invalid_type<>>{};
                  default:
                    break;
                }

                bug("unmapped unary operation");
              },
              ty);
        });
  }

  unimplemented("infer_type");
}

}  // namespace sema

}  // namespace via

#endif
