/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <via/config.h>
#include <via/types.h>
#include <variant>
#include "lexer/token.h"
#include "option.h"
#include "vm/value.h"

namespace via
{

namespace sema
{

class ConstValue final
{
 public:
  using nil_type = std::monostate;
  using int_type = Value::int_type;
  using float_type = Value::float_type;

  using Kind = Value::Kind;
  using Union = std::variant<nil_type, int_type, float_type, bool, std::string>;

 public:
  constexpr explicit ConstValue() : u(nil_type()) {}
  constexpr explicit ConstValue(int_type int_) : u(int_) {}
  constexpr explicit ConstValue(float_type float_) : u(float_) {}
  constexpr explicit ConstValue(bool boolean) : u(boolean) {}
  constexpr explicit ConstValue(std::string string) : u(string) {}

  static Option<ConstValue> fromToken(const Token& tok);

 public:
  constexpr Kind kind() const { return static_cast<Kind>(u.index()); }
  constexpr Union& data() { return u; }
  constexpr const Union& data() const { return u; }

  template <const Kind kind>
  constexpr auto value() const
  {
    return std::get<static_cast<usize>(kind)>(u);
  }

  constexpr bool compare(const ConstValue& other) const
  {
    return std::visit(
      [&other](auto&& lhs) -> bool {
        using T = std::decay_t<decltype(lhs)>;
        if (!std::holds_alternative<T>(other.u))
          return false;

        return lhs == std::get<T>(other.u);
      },
      u);
  }

  std::string toString() const;
  std::string dump() const;

 private:
  Union u;
};

}  // namespace sema

}  // namespace via
