// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#include "const_value.h"
#include <magic_enum/magic_enum.hpp>
#include "color.h"
#include "constexpr_stof.h"
#include "constexpr_stoi.h"

namespace via
{

namespace sema
{

Optional<ConstValue> ConstValue::fromToken(const Token& tok)
{
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
      if (auto val = stoi<ConstValue::int_type>(tok.toString()))
        return ConstValue(*val);
      break;
    case Token::Kind::FP:
      if (auto val = stof<ConstValue::float_type>(tok.toString()))
        return ConstValue(*val);
      break;
    default:
      break;
  }

  return nullopt;
}

String ConstValue::toString() const
{
  switch (kind()) {
    case Kind::Nil:
      return "nil";
    case Kind::Boolean:
      return value<Kind::Boolean>() ? "true" : "false";
    case Kind::Int:
      return std::to_string(value<Kind::Int>());
    case Kind::Float:
      return std::to_string(value<Kind::Float>());
    case Kind::String:
      return fmt::format("\"{}\"", value<Kind::String>());
    default:
      break;
  }

  return "<unknown-cv-type>";
}

String ConstValue::dump() const
{
  return fmt::format("{} [{} {}]",
                     applyANSI("constant", Fg::Magenta, Bg::Black, Style::Bold),
                     magic_enum::enum_name(kind()), toString());
}

}  // namespace sema

}  // namespace via
