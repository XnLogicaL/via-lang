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
#include "type_truthiness.h"

namespace via {

namespace sema {

namespace types {

template <typename... Args>
using any = Variant<types::nil_type,
                    types::bool_type,
                    types::int_type,
                    types::float_type,
                    types::string_type>;

};

template <typename T>
struct resolve_maybe_invalid {
  using type = std::conditional_t<types::is_valid_type_v<T>,
                                  typename T::type,
                                  types::nil_type>;
};

template <typename T>
using resolve_maybe_invalid_t = resolve_maybe_invalid<T>::type;

template <typename... Generics>
Optional<types::any<Generics...>> infer_type(Context& ctx,
                                             const ast::ExprNode* expr) {
  using types::UnOp;
  using any = types::any<Generics...>;

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

    return nullopt;
  } else if TRY_COERCE (const ast::NodeExprSym, sym, expr) {
    Frame& frame = ctx.stack.top();
    StringView symbol = sym->tok->to_string_view();

    // TODO: TEMPORARY IMPLEMENTATION
    if (auto lref = frame.get_local(symbol))
      return infer_type(ctx, lref->local.get_rval());

    bug("lookup failure");
  } else if TRY_COERCE (const ast::NodeExprUn, un, expr) {
    UnOp op = types::to_unop(un->op->kind);

    return infer_type(ctx, un->expr).and_then([&op](any&& ty) -> Optional<any> {
      return std::visit(
          [op](auto&& t) -> any {
            using T = std::decay_t<decltype(t)>;

            switch (op) {
              case UnOp::Neg:
                return resolve_maybe_invalid_t<
                    types::unary_result<UnOp::Neg, T>>{};
              case UnOp::Not:
                return types::unary_result_t<UnOp::Not, T>{};
              case UnOp::Bnot:
                return resolve_maybe_invalid_t<
                    types::unary_result<UnOp::Bnot, T>>{};
              default:
                break;
            }

            bug("unmapped unary operation");
            std::unreachable();
          },
          ty);
    });
  }

  unimplemented("infer_type");
  std::unreachable();
}

}  // namespace sema

}  // namespace via

#endif
