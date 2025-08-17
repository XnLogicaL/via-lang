// This file is a part of the via Programming Language project
// Copyright (C) 2024-2025 XnLogical - Licensed under GNU GPL v3.0

#ifndef VIA_CORE_CONST_VALUE_H_
#define VIA_CORE_CONST_VALUE_H_

#include <via/config.h>
#include <via/types.h>
#include "lexer/token.h"
#include "vm/value.h"

namespace via {

namespace sema {

class ConstValue final {
 public:
  using nil_type = monostate;
  using int_type = Value::int_type;
  using float_type = Value::float_type;
  using Kind = Value::Kind;
  using Union = Variant<nil_type, int_type, float_type, bool, String>;

 public:
  constexpr explicit ConstValue() : u(nil_type()) {}
  constexpr explicit ConstValue(int_type int_) : u(int_) {}
  constexpr explicit ConstValue(float_type float_) : u(float_) {}
  constexpr explicit ConstValue(bool boolean) : u(boolean) {}
  constexpr explicit ConstValue(String string) : u(string) {}

  static Optional<ConstValue> from_literal_token(const Token& tok);

 public:
  constexpr Kind kind() const { return static_cast<Kind>(u.index()); }
  constexpr Union& data() { return u; }
  constexpr const Union& data() const { return u; }

  constexpr bool compare(const ConstValue& other) const {
    // clang-format off
    return std::visit([&other](auto&& lhs) -> bool {
      using T = std::decay_t<decltype(lhs)>;
      if (!std::holds_alternative<T>(other.u))
        return false;

      return lhs == std::get<T>(other.u);
    }, u);
    // clang-format on
  }

 private:
  Union u;
};

}  // namespace sema

}  // namespace via

#endif
