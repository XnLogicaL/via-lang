/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <functional>
#include <optional>

namespace via
{

namespace util
{

template<typename T, typename... K>
class callable_once
{
    bool has_called = false;
    std::function<T(K...)> func;

public:
    callable_once(std::function<T(K...)> f)
        : func(std::move(f))
    {
    }

    std::optional<T> call(K... _Args)
    {
        if (has_called)
        {
            return std::nullopt;
        }

        has_called = true;
        return func(_Args...);
    }
};

} // namespace util

} // namespace via
