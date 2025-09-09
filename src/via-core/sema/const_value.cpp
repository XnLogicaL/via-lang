/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#include "const_value.h"
#include <magic_enum/magic_enum.hpp>
#include "ansi.h"
#include "constexpr_stof.h"
#include "constexpr_stoi.h"

namespace sema = via::sema;

via::Option<sema::ConstValue> sema::ConstValue::fromToken(const Token& tok)
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

std::string sema::ConstValue::toString() const
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
      return std::format("\"{}\"", value<String>());
    default:
      break;
  }

  return "<unknown-cv-type>";
}

std::string sema::ConstValue::dump() const
{
  return std::format("{} [{} {}]",
                     ansi("constant", Fg::Magenta, Bg::Black, Style::Bold),
                     magic_enum::enum_name(kind()), toString());
}
