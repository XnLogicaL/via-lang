/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <variant>
#include <via/config.h>
#include <via/types.h>
#include "lexer/token.h"
#include "support/option.h"
#include "support/utility.h"

namespace via {

#define FOR_EACH_VALUE_KIND(X)                                                           \
    X(NIL)                                                                               \
    X(INT)                                                                               \
    X(FLOAT)                                                                             \
    X(BOOL)                                                                              \
    X(STRING)                                                                            \
    X(FUNCTION)

enum class ValueKind : u8
{
    FOR_EACH_VALUE_KIND(DEFINE_ENUM)
};

DEFINE_TO_STRING(ValueKind, FOR_EACH_VALUE_KIND(DEFINE_CASE_TO_STRING))

namespace sema {

class ConstValue final
{
  public:
    using Union = std::variant<std::monostate, bool, i64, f64, std::string>;

  public:
    // clang-format off
    constexpr explicit ConstValue() : u(std::monostate{}) {}
    constexpr explicit ConstValue(bool boolean) : u(boolean) {}
    constexpr explicit ConstValue(i64 integer) : u(integer) {}
    constexpr explicit ConstValue(f64 float_) : u(float_) {}
    constexpr explicit ConstValue(std::string string) : u(string) {}
    // clang-format on

    static Option<ConstValue> from_token(const Token& tok);

  public:
    constexpr auto kind() const { return static_cast<ValueKind>(u.index()); }
    constexpr auto& data() { return u; }
    constexpr const auto& data() const { return u; }

    template <const ValueKind kind>
    constexpr auto value() const
    {
        return std::get<static_cast<size_t>(kind)>(u);
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
            u
        );
    }

    std::string to_string() const;
    std::string get_dump() const;

  private:
    Union u;
};

} // namespace sema
} // namespace via
