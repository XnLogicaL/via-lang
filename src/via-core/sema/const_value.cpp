// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "const_value.h"
#include "constexpr_stof.h"
#include "constexpr_stoi.h"

namespace via {

namespace sema {

Optional<ConstValue> ConstValue::from_literal_token(const Token& tok) {
  switch (tok.kind) {
    case Token::Kind::NIL:
      return ConstValue();
    case Token::Kind::TRUE:
      return ConstValue(true);
    case Token::Kind::FALSE:
      return ConstValue(false);
    case Token::Kind::INT:
    case Token::Kind::XINT:
    case Token::Kind::BINT:
      if (auto val = stoi<ConstValue::int_type>(tok.to_string()))
        return ConstValue(*val);
      break;
    case Token::Kind::FP:
      if (auto val = stof<ConstValue::float_type>(tok.to_string()))
        return ConstValue(*val);
      break;
    default:
      break;
  }

  return nullopt;
}

}  // namespace sema

}  // namespace via
