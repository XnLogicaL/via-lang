// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "type.h"

namespace via {

namespace sema {

types::any get_type(const Context& ctx, const ast::ExprNode* expr) {
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
  }

  VIA_BUG();
}

}  // namespace sema

}  // namespace via
