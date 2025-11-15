/* ===================================================== **
**  This file is a part of the via Programming Language  **
** ----------------------------------------------------- **
**           Copyright (C) XnLogicaL 2024-2025           **
**              Licensed under GNU GPLv3.0               **
** ----------------------------------------------------- **
**         https://github.com/XnLogicaL/via-lang         **
** ===================================================== */

#pragma once

#include <ranges>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>
#include <via/config.hpp>
#include "traits.hpp"

#define PASTE(a, b) a##b
#define STRING(X) #X

#define EXPAND(X) X
#define EXPAND_STRING(X) STRING(X)
#define EXPAND_AND_PASTE(A, B) PASTE(A, B)

// xmacro utils
#define DEFINE_ENUM(OP) OP,
#define DEFINE_CASE(OP, ...)                                                             \
    case OP:                                                                             \
        __VA_ARGS__
#define DEFINE_CASE_TO_STRING(OP)                                                        \
    case OP:                                                                             \
        return EXPAND_STRING(OP);

// Enum utils
#define DEFINE_TO_STRING(ENUM, ...)                                                      \
    constexpr std::string_view to_string(ENUM val) noexcept                              \
    {                                                                                    \
        using enum ENUM;                                                                 \
        switch (val) {                                                                   \
            __VA_ARGS__                                                                  \
        default:                                                                         \
            return "<error enum " #ENUM ">";                                             \
        }                                                                                \
    }

#define NO_COPY(TARGET)                                                                  \
    TARGET& operator=(const TARGET&) = delete;                                           \
    TARGET(const TARGET&) = delete;

#define IMPL_COPY(TARGET)                                                                \
    TARGET& operator=(const TARGET&);                                                    \
    TARGET(const TARGET&);

#define NO_MOVE(TARGET)                                                                  \
    TARGET& operator=(TARGET&&) = delete;                                                \
    TARGET(TARGET&&) = delete;

#define IMPL_MOVE(TARGET)                                                                \
    TARGET& operator=(TARGET&&);                                                         \
    TARGET(TARGET&&);

namespace via {
namespace detail {

template <typename Stream, typename T>
concept streamable_into = requires(Stream& stream, T a) {
    { stream << a } -> std::same_as<Stream&>;
};

} // namespace detail

template <std::ranges::range Range, std::invocable<std::ranges::range_value_t<Range>> Fn>
    requires std::convertible_to<
        std::invoke_result_t<Fn, std::ranges::range_value_t<Range>>,
        std::string>
constexpr std::string to_string(
    const Range& range,
    Fn callback,
    std::string_view open = "[",
    std::string_view close = "]",
    std::string_view delimiter = ","
)
{
    std::ostringstream oss;
    oss << open;

    auto it = range.begin();
    auto end = range.end();

    while (it != end) {
        oss << callback(*it);
        if (std::next(it) != end)
            oss << delimiter;
        ++it;
    }

    oss << close;
    return oss.str();
}

template <std::ranges::range Range>
    requires detail::
        streamable_into<std::ostringstream, std::ranges::range_value_t<Range>>
    constexpr std::string to_string(
        const Range& range,
        std::string_view open = "[",
        std::string_view close = "]",
        std::string_view delimiter = ","
    )
{
    std::ostringstream oss;
    oss << open;

    auto it = range.begin();
    auto end = range.end();

    while (it != end) {
        oss << *it;
        if (std::next(it) != end)
            oss << delimiter;
        ++it;
    }

    oss << close;
    return oss.str();
}

} // namespace via
