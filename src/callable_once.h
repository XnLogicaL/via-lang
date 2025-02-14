/* This file is a part of the via programming language at https://github.com/XnLogicaL/via-lang, see LICENSE for license information */

#pragma once

#include <functional>
#include <optional>

namespace via::utils {

template<typename T, typename... K>
class CallableOnce {
    bool has_called = false;
    std::function<T(K...)> func;

public:
    CallableOnce(std::function<T(K...)> f)
        : func(f)
    {
    }

    std::optional<const T &> call(K... _Args)
    {
        if (has_called)
            return std::nullopt;

        has_called = true;
        return func(_Args...);
    }
};

} // namespace via::utils
