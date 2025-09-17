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

namespace via {

namespace sema {

class ConstValue final
{
  public:
    enum class Kind
    {
        NIL,
        BOOL,
        INT,
        FLOAT,
        STRING,
    };

    using Union = std::variant<std::monostate, bool, i64, f64, std::string>;

  public:
    constexpr explicit ConstValue() :
        u(std::monostate{})
    {}
    constexpr explicit ConstValue(bool boolean) :
        u(boolean)
    {}
    constexpr explicit ConstValue(i64 integer) :
        u(integer)
    {}
    constexpr explicit ConstValue(f64 float_) :
        u(float_)
    {}
    constexpr explicit ConstValue(std::string string) :
        u(string)
    {}

    static Option<ConstValue> from_token(const Token& tok);

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
