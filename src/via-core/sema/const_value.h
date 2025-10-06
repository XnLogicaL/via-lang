/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>
#include <variant>
#include <via/config.h>
#include "lexer/token.h"
#include "support/utility.h"

namespace via {

#define FOR_EACH_VALUE_KIND(X)                                                           \
    X(NIL)                                                                               \
    X(INT)                                                                               \
    X(FLOAT)                                                                             \
    X(BOOL)                                                                              \
    X(STRING)                                                                            \
    X(FUNCTION)

enum class ValueKind : uint8_t
{
    FOR_EACH_VALUE_KIND(DEFINE_ENUM)
};

DEFINE_TO_STRING(ValueKind, FOR_EACH_VALUE_KIND(DEFINE_CASE_TO_STRING))

namespace sema {

class ConstValue final
{
  public:
    using Union = std::variant<std::monostate, bool, int64_t, double_t, std::string>;

  public:
    // clang-format off
    constexpr explicit ConstValue() : m_data(std::monostate{}) {}
    constexpr explicit ConstValue(bool boolean) : m_data(boolean) {}
    constexpr explicit ConstValue(int64_t integer) : m_data(integer) {}
    constexpr explicit ConstValue(double_t float_) : m_data(float_) {}
    constexpr explicit ConstValue(std::string string) : m_data(string) {}
    // clang-format on

    static std::optional<ConstValue> from_token(const Token& tok);

  public:
    constexpr auto kind() const { return static_cast<ValueKind>(m_data.index()); }
    constexpr auto& data() { return m_data; }
    constexpr const auto& data() const { return m_data; }

    template <const ValueKind kind>
    constexpr auto value() const
    {
        return std::get<static_cast<size_t>(kind)>(m_data);
    }

    constexpr bool compare(const ConstValue& other) const
    {
        return std::visit(
            [&other](auto&& lhs) -> bool {
                using T = std::decay_t<decltype(lhs)>;
                if (!std::holds_alternative<T>(other.m_data))
                    return false;

                return lhs == std::get<T>(other.m_data);
            },
            m_data
        );
    }

    std::string to_string() const;
    std::string get_dump() const;

  private:
    Union m_data;
};

} // namespace sema
} // namespace via
