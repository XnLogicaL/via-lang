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

Option<ConstValue> ConstValue::fromToken(const Token& tok)
{
  switch (tok.kind) {
    case Token::Kind::LIT_NIL:
      return ConstValue();
    case Token::Kind::LIT_TRUE:
      return ConstValue(true);
    case Token::Kind::LIT_FALSE:
      return ConstValue(false);
    case Token::Kind::LIT_INT:
    case Token::Kind::LIT_XINT:
    case Token::Kind::LIT_BINT:
      if (auto val = stoi<ConstValue::int_type>(tok.toString()))
        return ConstValue(*val);
      break;
    case Token::Kind::LIT_FLOAT:
      if (auto val = stof<ConstValue::float_type>(tok.toString()))
        return ConstValue(*val);
      break;
    default:
      break;
  }

  return nullopt;
}

std::string ConstValue::toString() const
{
  using enum Kind;

  switch (kind()) {
    case Nil:
      return "nil";
    case Boolean:
      return value<Boolean>() ? "true" : "false";
    case Int:
      return std::to_string(value<Int>());
    case Float:
      return std::to_string(value<Float>());
    case String:
      return fmt::format("\"{}\"", value<String>());
    default:
      break;
  }

  return "<unknown-cv-type>";
}

std::string ConstValue::dump() const
{
  return fmt::format(
    "{} [{} {}]", ansiFormat("constant", Fg::Magenta, Bg::Black, Style::Bold),
    magic_enum::enum_name(kind()), toString());
}

}  // namespace sema

}  // namespace via
